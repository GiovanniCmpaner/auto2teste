
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>
#include <numeric>

#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "Neural.hpp"

namespace Neural
{
    namespace
    {
        auto inputValues{std::vector<float>{}};
        auto outputValues{std::vector<float>{}};
        auto doInference{false};

        auto modelBuffer{std::vector<char>{}};
        auto errorReporter{static_cast<tflite::ErrorReporter *>(nullptr)};
        auto model{static_cast<const tflite::Model *>(nullptr)};
        auto interpreter{static_cast<tflite::MicroInterpreter *>(nullptr)};
        auto tensorArena{std::array<uint8_t, 2 * 1024>{}};
        auto inputTensor{static_cast<TfLiteTensor *>(nullptr)};
        auto outputTensor{static_cast<TfLiteTensor *>(nullptr)};
        auto inputSize{0};
        auto outputSize{0};

        auto load() -> void
        {
            if (not SPIFFS.exists("/model.tflite"))
            {
                log_d("model file not found");
                return;
            }

            SPIFFS.remove("/model_loading.tflite");
            SPIFFS.rename("/model.tflite", "/model_loading.tflite");

            auto file{SPIFFS.open("/model_loading.tflite", FILE_READ)};
            if (not file)
            {
                log_e("model file error");
                return;
            }

            modelBuffer.resize(file.size());
            file.readBytes(modelBuffer.data(), modelBuffer.size());

            // Set up logging. Google style is to avoid globals or statics because of
            // lifetime uncertainty, but since this has a trivial destructor it's okay.
            // NOLINTNEXTLINE(runtime-global-variables)
            static auto micro_errorReporter{tflite::MicroErrorReporter{}};
            errorReporter = &micro_errorReporter;

            // Map the model into a usable data structure. This doesn't involve any
            // copying or parsing, it's a very lightweight operation.
            auto model{tflite::GetModel(modelBuffer.data())};
            if (model->version() != TFLITE_SCHEMA_VERSION)
            {
                errorReporter->Report(
                    "Model provided is schema version %d not equal "
                    "to supported version %d.",
                    model->version(), TFLITE_SCHEMA_VERSION);
                return;
            }

            // This pulls in all the operation implementations we need.
            // NOLINTNEXTLINE(runtime-global-variables)
            static auto resolver{tflite::ops::micro::AllOpsResolver{}};

            // Build an interpreter to run the model with.
            static auto static_interpreter{tflite::MicroInterpreter{model, resolver, tensorArena.data(), tensorArena.size(), errorReporter}};
            interpreter = &static_interpreter;

            // Allocate memory from the tensor_arena for the model's tensors.
            auto allocate_status{interpreter->AllocateTensors()};
            if (allocate_status != kTfLiteOk)
            {
                errorReporter->Report("AllocateTensors() failed");
                return;
            }

            // Obtain pointers to the model's inputTensor and outputTensor tensors.
            inputTensor = interpreter->input(0);
            outputTensor = interpreter->output(0);

            inputSize = inputTensor->dims->data[1];
            outputSize = outputTensor->dims->data[1];

            log_d("inputSize = %d", inputSize);
            log_d("outputSize = %d", outputSize);

            SPIFFS.rename("/model_loading.tflite", "/model.tflite");
        }

        auto inference() -> void
        {
            if (interpreter == nullptr)
            {
                return;
            }

            std::memcpy(inputTensor->data.f, Neural::inputValues.data(), Neural::inputValues.size());

            // Run inference, and report any error
            const auto invoke_status{interpreter->Invoke()};
            if (invoke_status != kTfLiteOk)
            {
                errorReporter->Report("Invoke failed on inputs");
                return;
            }

            std::memcpy(Neural::outputValues.data(), outputTensor->data.f, Neural::outputValues.size());
        }

    } // namespace

    auto init() -> void
    {
        log_d("begin");

        SPIFFS.begin(true);

        Neural::load();

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        if (doInference)
        {
            Neural::inference();
            doInference = false;
        }
    }

    auto inputs(const std::vector<float> &inputValues) -> bool
    {
        if (Neural::inputTensor == nullptr or inputValues.size() != inputSize)
        {
            return false;
        }

        Neural::inputValues = inputValues;
        doInference = true;

        return true;
    }

    auto outputs(std::vector<float> *outputValues) -> bool
    {
        if (Neural::outputTensor == nullptr or outputValues->size() != outputSize)
        {
            return false;
        }

        *outputValues = Neural::outputValues;

        return true;
    }

} // namespace Neural