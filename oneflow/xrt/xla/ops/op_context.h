#ifndef ONEFLOW_XRT_XLA_OPS_OP_CONTEXT_H_
#define ONEFLOW_XRT_XLA_OPS_OP_CONTEXT_H_

#include "oneflow/core/common/data_type.h"
#include "oneflow/core/common/protobuf.h"
#include "oneflow/core/common/shape.h"
#include "oneflow/xrt/argument.h"
#include "oneflow/xrt/kernel/op_context.h"
#include "oneflow/xrt/types.h"
#include "oneflow/xrt/utility/stl.h"
#include "oneflow/xrt/xrt.pb.h"

#include "tensorflow/compiler/xla/client/xla_builder.h"
#include "tensorflow/compiler/xla/shape.h"

namespace oneflow {
namespace xrt {
namespace mola {

class XlaValue {
 public:
  XlaValue() : initialized_(false) {}
  // Construct from Constant shape.
  static XlaValue Constant(const xla::Shape shape);
  // Construct from XlaOp handle.
  static XlaValue XlaOp(const xla::XlaOp handle);

  // Return the XlaOp handle if the builder is matched with the handle.
  xla::XlaOp AsXlaOp(xla::XlaBuilder *builder) const;

  friend class XlaOpContext;

 private:
  bool initialized_;
  // XlaOp handle should be initialized if the value is
  // constructed from another XlaOp.
  xla::XlaOp handle_;
  // Shape of the xla value.
  xla::Shape shape_;
};

class XlaOpContext : public OpContext {
 public:
  struct Param {
    // XlaBuilder to compile the XlaComputation
    xla::XlaBuilder *builder;

    XrtDevice device;
    // Config proto related to the operator
    const PbMessage *message;
    // Input operands
    util::Map<Argument, XlaValue> inputs;
    int num_outputs;

    util::Map<std::string, Argument> arguments;
  };

  explicit XlaOpContext(const Param &param) : OpContext(*param.message), param_(param) {}

  virtual ~XlaOpContext() = default;

  const XrtDevice &device() const { return param_.device; }
  // Return XlaBuilder
  xla::XlaBuilder *builder() const;

  // Return input named `name` as XlaOp
  xla::XlaOp Input(const std::string &name);
  xla::XlaOp Input(const Argument &arg);
  // Return output named `name` as XlaOp
  xla::XlaOp Output(const std::string &name);
  xla::XlaOp Output(const Argument &arg);

  int num_inputs() const { return param_.inputs.size(); }
  int num_outputs() const { return param_.num_outputs; }
  // Return inputs as XlaValues
  const util::Map<Argument, XlaValue> &inputs() const { return param_.inputs; }
  // Return output as XlaValues
  const util::Map<Argument, XlaValue> &outputs() const { return outputs_; }

  // Setup the output `output_name` with XlaOp
  void SetOutput(const std::string &name, const xla::XlaOp &handle);
  // Setup the output `output_name` with XlaValue
  void SetOutput(const std::string &name, const XlaValue &handle);

  // Return input `name` shape as Shape
  Shape InputShape(const std::string &name) const;
  // Return output `name` shape as Shape
  Shape OutputShape(const std::string &name) const;

  // Input data type
  DataType InputType(const std::string &name) const;
  // Output data type
  DataType OutputType(const std::string &name) const;

  const Param &param() const { return param_; }

 private:
  XlaOpContext() = delete;
  Argument ArgumentFromKey(const std::string &key) const;

  Param param_;
  // Output operands
  util::Map<Argument, XlaValue> outputs_;
};

}  // namespace mola
}  // namespace xrt
}  // namespace oneflow

#endif  // ONEFLOW_XRT_XLA_OPS_OP_CONTEXT_H_