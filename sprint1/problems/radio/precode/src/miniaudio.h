#ifndef MINIAUDIO_H
#define MINIAUDIO_H

#include <cstddef>
#include <cstdint>

// Типы результатов
typedef enum ma_result {
    MA_SUCCESS = 0,
    MA_ERROR = -1
} ma_result;

// Типы устройств
typedef enum ma_device_type {
    ma_device_type_playback,
    ma_device_type_capture,
    ma_device_type_duplex
} ma_device_type;

// Форматы
typedef enum ma_format {
    ma_format_u8,
    ma_format_s16,
    ma_format_s24,
    ma_format_s32,
    ma_format_f32
} ma_format;

// Бэкенды
typedef enum ma_backend {
    ma_backend_alsa,
    ma_backend_pulseaudio,
    ma_backend_wasapi,
    ma_backend_null
} ma_backend;

// Состояния
typedef enum ma_device_state {
    ma_device_state_started,
    ma_device_state_stopped,
    ma_device_state_error
} ma_device_state;

typedef uint32_t ma_uint32;
typedef uint64_t ma_uint64;
typedef int32_t ma_int32;

// Предварительные объявления
struct ma_context;
struct ma_device;
struct ma_engine;

// Типы callback'ов
typedef void (*ma_device_callback_proc)(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

// Структура для ID устройства
struct ma_device_id {
    void* pData;
};

// Конфигурация устройства - capture
struct ma_capture_config {
    ma_device_id* pDeviceID;
    ma_format format;
    ma_uint32 channels;
};

// Конфигурация устройства - playback
struct ma_playback_config {
    ma_device_id* pDeviceID;
    ma_format format;
    ma_uint32 channels;
};

// Конфигурация устройства
struct ma_device_config {
    ma_device_type deviceType;
    ma_uint32 sampleRate;
    ma_uint32 bufferSizeInFrames;
    ma_device_callback_proc dataCallback;
    void* pUserData;
    ma_capture_config capture;
    ma_playback_config playback;
};

// Конфигурация контекста
struct ma_context_config {
    ma_backend backend;
    void* pAllocationCallbacks;
};

// Контекст
struct ma_context {
    void* pData;
};

// Устройство
struct ma_device {
    ma_device_config config;
    void* pUserData;
    void* pCallbackData;
    ma_context* pContext;
};

// Конфигурация движка
struct ma_engine_config {
    void* pDevice;
    void* pResourceManager;
    void* pAssetManager;
    int noAutoStart;
    int mono;
    ma_uint32 sampleRate;
};

// Движок
struct ma_engine {
    void* pData;
    ma_device* pDevice;
};

// Функции инициализации
static inline ma_device_config ma_device_config_init(ma_device_type deviceType) {
    ma_device_config config = {};
    config.deviceType = deviceType;
    config.sampleRate = 48000;
    config.bufferSizeInFrames = 1024;
    return config;
}

static inline ma_context_config ma_context_config_init() {
    ma_context_config config = {};
    return config;
}

static inline ma_engine_config ma_engine_config_init() {
    ma_engine_config config = {};
    config.sampleRate = 48000;
    return config;
}

// Функции контекста
static inline ma_result ma_context_init(ma_backend backend, ma_context_config* pConfig, ma_context* pContext) {
    (void)backend; (void)pConfig;
    if (pContext) {
        pContext->pData = (void*)1;
    }
    return MA_SUCCESS;
}

static inline void ma_context_uninit(ma_context* pContext) {
    (void)pContext;
}

// Функции устройства
static inline ma_result ma_device_init(ma_context* pContext, ma_device_config* pConfig, ma_device* pDevice) {
    (void)pContext;
    if (pDevice && pConfig) {
        pDevice->config = *pConfig;
        pDevice->pUserData = pConfig->pUserData;
        pDevice->pContext = pContext;
    }
    return MA_SUCCESS;
}

static inline void ma_device_uninit(ma_device* pDevice) {
    (void)pDevice;
}

static inline ma_result ma_device_start(ma_device* pDevice) {
    (void)pDevice;
    return MA_SUCCESS;
}

static inline ma_result ma_device_stop(ma_device* pDevice) {
    (void)pDevice;
    return MA_SUCCESS;
}

static inline ma_device_state ma_device_get_state(ma_device* pDevice) {
    (void)pDevice;
    return ma_device_state_started;
}

// Функции для работы с байтами
static inline size_t ma_get_bytes_per_frame(ma_format format, ma_uint32 channels) {
    (void)format;
    return channels;  // Для u8 = 1 байт на канал
}

// Функции движка
static inline ma_result ma_engine_init(ma_engine_config* pConfig, ma_engine* pEngine) {
    (void)pConfig;
    if (pEngine) {
        pEngine->pData = (void*)1;
    }
    return MA_SUCCESS;
}

static inline void ma_engine_uninit(ma_engine* pEngine) {
    (void)pEngine;
}

static inline ma_result ma_engine_start(ma_engine* pEngine) {
    (void)pEngine;
    return MA_SUCCESS;
}

#endif // MINIAUDIO_H
