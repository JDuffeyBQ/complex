#pragma once

#include "complex/Filter/IFilter.hpp"
#include "complex/complex_export.hpp"

namespace complex
{
class COMPLEX_EXPORT TestFilter1 : public IFilter
{
public:
  TestFilter1() = default;
  ~TestFilter1() noexcept override = default;

  TestFilter1(const TestFilter1&) = delete;
  TestFilter1(TestFilter1&&) = delete;

  TestFilter1& operator=(const TestFilter1&) = delete;
  TestFilter1& operator=(TestFilter1&&) = delete;

  /**
   * @brief
   * @return
   */
  [[nodiscard]] std::string name() const override;

  /**
   * @brief
   * @return
   */
  [[nodiscard]] Uuid uuid() const override;

  /**
   * @brief
   * @return
   */
  [[nodiscard]] std::string humanName() const override;

  /**
   * @brief
   * @return
   */
  [[nodiscard]] Parameters parameters() const override;

private:
  /**
   * @brief
   * @param data
   * @param args
   * @param messageHandler
   * @return
   */
  Result<OutputActions> preflightImpl(const DataStructure& data, const Arguments& args, const MessageHandler& messageHandler) const override;

  /**
   * @brief
   * @param data
   * @param args
   * @param messageHandler
   * @return
   */
  Result<> executeImpl(DataStructure& data, const Arguments& args, const MessageHandler& messageHandler) const override;
};
} // namespace complex