#include <catch2/catch.hpp>

#include "ComplexCore/ComplexCore_test_dirs.hpp"
#include "ComplexCore/Filters/CropImageGeometry.hpp"

#include "complex/Common/StringLiteral.hpp"
#include "complex/DataStructure/DataArray.hpp"
#include "complex/UnitTest/UnitTestCommon.hpp"
#include "complex/Utilities/DataGroupUtilities.hpp"
#include "complex/Utilities/FilterUtilities.hpp"

using namespace complex;

namespace
{

struct CompareDataArrayFunctor
{
  template <typename T>
  void operator()(const IDataArray& left, const IDataArray& right, usize start = 0)
  {
    UnitTest::CompareDataArrays<T>(left, right, start);
  }
};

DataStructure CreateDataStructure()
{
  DataStructure dataGraph;
  DataGroup* topLevelGroup = DataGroup::Create(dataGraph, Constants::k_SmallIN100);
  DataGroup* scanData = DataGroup::Create(dataGraph, Constants::k_EbsdScanData, topLevelGroup->getId());

  ImageGeom* imageGeom = ImageGeom::Create(dataGraph, Constants::k_ImageGeometry, scanData->getId());
  imageGeom->setSpacing({0.25f, 0.55f, 1.86});
  imageGeom->setOrigin({0.0f, 20.0f, 66.0f});
  SizeVec3 imageGeomDims = {40, 60, 80};
  imageGeom->setDimensions(imageGeomDims); // Listed from slowest to fastest (Z, Y, X)
  auto* cellData = AttributeMatrix::Create(dataGraph, ImageGeom::k_CellDataName, imageGeom->getId());
  auto imageDimsArray = imageGeomDims.toArray();
  AttributeMatrix::ShapeType cellDataDims{imageDimsArray.crbegin(), imageDimsArray.crend()};
  cellData->setShape(cellDataDims);
  imageGeom->setCellData(*cellData);

  Int32Array* phases_data = UnitTest::CreateTestDataArray<int32>(dataGraph, "Phases", cellDataDims, {1}, cellData->getId());

  return dataGraph;
}
} // namespace

TEST_CASE("ComplexCore::CropImageGeometry(Instantiate)", "[ComplexCore][CropImageGeometry]")
{
  const std::vector<uint64> k_MinVector{0, 0, 0};
  const std::vector<uint64> k_MaxVector{0, 0, 0};

  static constexpr bool k_UpdateOrigin = false;
  const DataPath k_ImageGeomPath({Constants::k_SmallIN100, Constants::k_EbsdScanData, Constants::k_ImageGeometry});
  const DataPath k_NewImageGeomPath({Constants::k_SmallIN100, "New Image Geom"});
  static constexpr bool k_RenumberFeatures = false;
  const DataPath k_FeatureIdsPath({Constants::k_SmallIN100, Constants::k_EbsdScanData, Constants::k_FeatureIds});

  CropImageGeometry filter;
  DataStructure ds = CreateDataStructure();
  Arguments args;

  args.insert(CropImageGeometry::k_MinVoxel_Key, std::make_any<std::vector<uint64>>(k_MinVector));
  args.insert(CropImageGeometry::k_MaxVoxel_Key, std::make_any<std::vector<uint64>>(k_MaxVector));
  args.insert(CropImageGeometry::k_UpdateOrigin_Key, std::make_any<bool>(k_UpdateOrigin));
  args.insert(CropImageGeometry::k_ImageGeom_Key, std::make_any<DataPath>(k_ImageGeomPath));
  args.insert(CropImageGeometry::k_NewImageGeom_Key, std::make_any<DataPath>(k_NewImageGeomPath));
  args.insert(CropImageGeometry::k_RenumberFeatures_Key, std::make_any<bool>(k_RenumberFeatures));
  args.insert(CropImageGeometry::k_FeatureIds_Key, std::make_any<DataPath>(k_FeatureIdsPath));
  args.insert(CropImageGeometry::k_RemoveOriginalGeometry_Key, std::make_any<bool>(true));

  auto result = filter.execute(ds, args);
  COMPLEX_RESULT_REQUIRE_VALID(result.result);
}

TEST_CASE("ComplexCore::CropImageGeometry(Execute_Filter)", "[ComplexCore][CropImageGeometry]")
{
  const std::vector<uint64> k_MinVector{10, 15, 0};
  const std::vector<uint64> k_MaxVector{60, 40, 50};

  static constexpr bool k_UpdateOrigin = false;
  const DataPath k_ImageGeomPath({Constants::k_DataContainer});
  const DataPath k_NewImageGeomPath({"7_0_Cropped_ImageGeom"});
  DataPath destCellDataPath = k_NewImageGeomPath.createChildPath(Constants::k_CellData);
  static constexpr bool k_RenumberFeatures = true;
  const DataPath k_FeatureIdsPath({Constants::k_DataContainer, Constants::k_CellData, Constants::k_FeatureIds});
  const DataPath k_CellFeatureAMPath({Constants::k_DataContainer, Constants::k_CellFeatureData});
  DataPath k_DestCellFeatureDataPath = k_NewImageGeomPath.createChildPath(Constants::k_CellFeatureData);

  CropImageGeometry filter;
  // Read the Small IN100 Data set
  auto baseDataFilePath = fs::path(fmt::format("{}/6_5_test_data_1.dream3d", unit_test::k_TestDataSourceDir));
  DataStructure ds = UnitTest::LoadDataStructure(baseDataFilePath);
  Arguments args;

  args.insert(CropImageGeometry::k_MinVoxel_Key, std::make_any<std::vector<uint64>>(k_MinVector));
  args.insert(CropImageGeometry::k_MaxVoxel_Key, std::make_any<std::vector<uint64>>(k_MaxVector));
  args.insert(CropImageGeometry::k_UpdateOrigin_Key, std::make_any<bool>(k_UpdateOrigin));
  args.insert(CropImageGeometry::k_ImageGeom_Key, std::make_any<DataPath>(k_ImageGeomPath));
  args.insert(CropImageGeometry::k_NewImageGeom_Key, std::make_any<DataPath>(k_NewImageGeomPath));
  args.insert(CropImageGeometry::k_RenumberFeatures_Key, std::make_any<bool>(k_RenumberFeatures));
  args.insert(CropImageGeometry::k_FeatureIds_Key, std::make_any<DataPath>(k_FeatureIdsPath));
  args.insert(CropImageGeometry::k_CellFeatureAttributeMatrix_Key, std::make_any<DataPath>(k_CellFeatureAMPath));
  args.insert(CropImageGeometry::k_RemoveOriginalGeometry_Key, std::make_any<bool>(false));

  const auto oldDimensions = ds.getDataRefAs<ImageGeom>(k_ImageGeomPath).getDimensions();

  auto result = filter.execute(ds, args);
  COMPLEX_RESULT_REQUIRE_VALID(result.result);

  {
    // Write out the DataStructure for later viewing/debugging
    Result<H5::FileWriter> ioResult = H5::FileWriter::CreateFile(fmt::format("{}/crop_image_geom_test.dream3d", unit_test::k_BinaryDir));
    H5::FileWriter fileWriter = std::move(ioResult.value());
    herr_t err = ds.writeHdf5(fileWriter);
    REQUIRE(err >= 0);
  }

  auto& newImageGeom = ds.getDataRefAs<ImageGeom>(k_NewImageGeomPath);
  auto newDimensions = newImageGeom.getDimensions();

  for(usize i = 0; i < 3; i++)
  {
    REQUIRE(newDimensions[i] == (k_MaxVector[i] - k_MinVector[i] + 1));
  }

  DataPath exemplarGeoPath({"6_5_Cropped_ImageGeom"});
  DataPath exemplarCellDataPath = exemplarGeoPath.createChildPath(Constants::k_CellData);
  DataPath exemplarCellFeatureDataPath = exemplarGeoPath.createChildPath(Constants::k_CellFeatureData);

  // check the data arrays
  const auto exemplarCellDataArrays = GetAllChildArrayDataPaths(ds, exemplarCellDataPath).value();
  const auto calculatedCellDataArrays = GetAllChildArrayDataPaths(ds, destCellDataPath).value();
  for(usize i = 0; i < exemplarCellDataArrays.size(); ++i)
  {
    const IDataArray& exemplarArray = ds.getDataRefAs<IDataArray>(exemplarCellDataArrays[i]);
    const IDataArray& calculatedArray = ds.getDataRefAs<IDataArray>(calculatedCellDataArrays[i]);
    ::ExecuteDataFunction(CompareDataArrayFunctor{}, exemplarArray.getDataType(), exemplarArray, calculatedArray);
  }

  const auto exemplarFeatureDataArrays = GetAllChildArrayDataPaths(ds, exemplarCellFeatureDataPath).value();
  const auto calculatedFeatureDataArrays = GetAllChildArrayDataPaths(ds, k_DestCellFeatureDataPath).value();
  for(usize i = 0; i < exemplarFeatureDataArrays.size(); ++i)
  {
    const IDataArray& exemplarArray = ds.getDataRefAs<IDataArray>(exemplarFeatureDataArrays[i]);
    const IDataArray& calculatedArray = ds.getDataRefAs<IDataArray>(calculatedFeatureDataArrays[i]);
    ExecuteDataFunction(CompareDataArrayFunctor{}, exemplarArray.getDataType(), exemplarArray, calculatedArray);
  }
}
