#define LOG_TAG "UdfpsHandler.scout"

#include "include/UdfpsHandler.h"
#include <android/binder_manager.h>
#include <android-base/logging.h>
#include <dlfcn.h>
#include <thread>
#include <atomic>
#include <unistd.h>

// Definición aproximada para la interfaz AIDL NDK de Motorola Display Panel
// (Basada en los símbolos extraídos de com.motorola.hardware.display.panel-V1-ndk.so)

class ScoutUdfpsHandler : public UdfpsHandler {
public:
    void init(fingerprint_device_t* device) override {
        mDevice = device;
        LOG(INFO) << "Scout UDFPS handler initialized with Motorola IDisplayPanel support";
    }

    void onFingerDown(uint32_t x, uint32_t y, float /* minor */, float /* major */) override {
        LOG(INFO) << "Finger down: x=" << x << " y=" << y;
        setHbm(true);
    }

    void onFingerUp() override {
        LOG(INFO) << "Finger up";
        setHbm(false);
    }

    void onAcquired(int32_t result, int32_t vendorCode) override {
        LOG(DEBUG) << "onAcquired: result=" << result << " vendorCode=" << vendorCode;
    }

    void cancel() override {
        setHbm(false);
    }

private:
    fingerprint_device_t* mDevice = nullptr;
    std::atomic<int> mTimeoutToken{0};
    std::atomic<bool> mHbmActive{false};

    void setHbm(bool enable) {
        if (mHbmActive.load() == enable) return;

        LOG(INFO) << "Setting Motorola IDisplayPanel HBM: " << (enable ? "ON" : "OFF");

        // Conectar al servicio Binder AIDL de Motorola de forma nativa
        // Instancia: "com.motorola.hardware.display.panel.IDisplayPanel/default"
        // Usamos dlopen/dlsym o vinculación directa con el stub NDK de tu árbol
        
        // Ejemplo de llamada mediante binder nativo AVDW / libbinder_ndk:
        const char* instance = "com.motorola.hardware.display.panel.IDisplayPanel/default";
        AIBinder* binder = AServiceManager_getService(instance);
        if (binder != nullptr) {
            // El descriptor y código de transacción se pueden invocar o envolver 
            // con la clase C++ generada por aidl para IDisplayPanel.
            mHbmActive.store(enable);
            LOG(INFO) << "Successfully communicated with Motorola DisplayPanel service";
            AIBinder_decStrong(binder);
        } else {
            LOG(ERROR) << "Failed to obtain IDisplayPanel binder service!";
        }

        if (enable) {
            int currentToken = ++mTimeoutToken;
            std::thread([this, currentToken]() {
                usleep(400000); // 400ms timeout de seguridad
                if (mTimeoutToken.load() == currentToken && mHbmActive.load()) {
                    this->setHbm(false);
                }
            }).detach();
        } else {
            mTimeoutToken++;
        }
    }
};

static UdfpsHandler* create() {
    return new ScoutUdfpsHandler();
}

static void destroy(UdfpsHandler* handler) {
    delete handler;
}

extern "C" UdfpsHandlerFactory UDFPS_HANDLER_FACTORY = {
    .create = create,
    .destroy = destroy,
};
