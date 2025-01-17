#include "DeleteData.hpp"

#include "complex/DataStructure/DataArray.hpp"
#include "complex/Filter/Actions/DeleteDataAction.hpp"
#include "complex/Parameters/DataPathSelectionParameter.hpp"

namespace complex
{
namespace
{
} // namespace

std::string DeleteData::name() const
{
  return FilterTraits<DeleteData>::name;
}

std::string DeleteData::className() const
{
  return FilterTraits<DeleteData>::className;
}

Uuid DeleteData::uuid() const
{
  return FilterTraits<DeleteData>::uuid;
}

std::string DeleteData::humanName() const
{
  return "Delete Data";
}

//------------------------------------------------------------------------------
std::vector<std::string> DeleteData::defaultTags() const
{
  return {"#Core", "#Memory Management", "#Remove Data", "#Delete Data"};
}

Parameters DeleteData::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Required Input Data Objects"});
  params.insert(std::make_unique<DataPathSelectionParameter>(k_DataPath_Key, "DataPath to remove", "The complete path to the DataObject to be removed", DataPath{}));
  return params;
}

IFilter::UniquePointer DeleteData::clone() const
{
  return std::make_unique<DeleteData>();
}

IFilter::PreflightResult DeleteData::preflightImpl(const DataStructure& data, const Arguments& args, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const
{
  auto dataArrayPath = args.value<DataPath>(k_DataPath_Key);

  auto action = std::make_unique<DeleteDataAction>(dataArrayPath);

  OutputActions actions;
  actions.actions.push_back(std::move(action));

  return {std::move(actions)};
}

Result<> DeleteData::executeImpl(DataStructure& data, const Arguments& args, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const
{
  return {};
}
} // namespace complex
