#include "my_camera.h"
#include "config.h"

#define TAG "OV2640"

camera_config_t camera_config {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 15,               //0-63 lower numbers are higher quality
    .fb_count = 1, // if more than one i2s runs in continous mode.  Use only with jpeg
    .fb_location = CAMERA_FB_IN_DRAM
};

void OV2640::run(void)
{
    if(fb)
        esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();
}

void OV2640::runIfNeeded(void)
{
    if(!fb)
        run();
}

int OV2640::getWidth(void)
{
    runIfNeeded();
    return fb->width;
}

int OV2640::getHeight(void)
{
    runIfNeeded();
    return fb->height;
}

size_t OV2640::getSize(void)
{
    runIfNeeded();
    return fb->len;
}

uint8_t *OV2640::getfb(void)
{
    runIfNeeded();
    return fb->buf;
}

framesize_t OV2640::getFrameSize(void)
{
    return _camera_config.frame_size;
}

void OV2640::setFrameSize(framesize_t size)
{
    _camera_config.frame_size = size;
}

pixformat_t OV2640::getPixelFormat(void)
{
    return _camera_config.pixel_format;
}

void OV2640::setPixelFormat(pixformat_t format)
{
    switch (format)
    {
    case PIXFORMAT_RGB565:
    case PIXFORMAT_YUV422:
    case PIXFORMAT_GRAYSCALE:
    case PIXFORMAT_JPEG:
        _camera_config.pixel_format = format;
        break;
    default:
        _camera_config.pixel_format = PIXFORMAT_GRAYSCALE;
        break;
    }
}


esp_err_t OV2640::init(camera_config_t config) {
//    memset(&_camera_config, 0, sizeof(_camera_config));
//    memcpy(&_camera_config, &config, sizeof(config));

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        printf("Camera probe failed with error 0x%x", err);
        m_camera_state = false;
        return err;
    }
    // ESP_ERROR_CHECK(gpio_install_isr_service(0));
    m_camera_state = true;
    return ESP_OK;
}

bool OV2640::getInitState() {
    return m_camera_state;
}
