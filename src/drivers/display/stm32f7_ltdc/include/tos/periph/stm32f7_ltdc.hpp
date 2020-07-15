#pragma once

#include <stm32f7xx_hal_conf.h>
#include <stm32f7xx_hal_ltdc.h>

namespace tos::stm32::f7 {
class ltdc_layer {
public:
    void setup() {

        //
        //    /* Layer1 Configuration
        //    ------------------------------------------------------*/
        //
        //    /* Windowing configuration */
        //    /* In this case all the active display area is used to display a picture
        //    then :
        //       Horizontal start = horizontal synchronization + Horizontal back porch =
        //       43 Vertical start   = vertical synchronization + vertical back porch = 12
        //       Horizontal stop = Horizontal start + window width -1 = 43 + 480 -1
        //       Vertical stop   = Vertical start + window height -1  = 12 + 272 -1 */
        //    pLayerCfg.WindowX0 = 0;
        //    pLayerCfg.WindowX1 = 480;
        //    pLayerCfg.WindowY0 = 0;
        //    pLayerCfg.WindowY1 = 272;
        //
        //    /* Pixel Format configuration*/
        //    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
        //
        //    /* Start Address configuration : frame buffer is located at FLASH memory */
        //    pLayerCfg.FBStartAdress = (uint32_t)&RGB565_480x272;
        //
        //    /* Alpha constant (255 == totally opaque) */
        //    pLayerCfg.Alpha = 255;
        //
        //    /* Default Color configuration (configure A,R,G,B component values) : no
        //    background
        //     * color */
        //    pLayerCfg.Alpha0 = 0; /* fully transparent */
        //    pLayerCfg.Backcolor.Blue = 0;
        //    pLayerCfg.Backcolor.Green = 0;
        //    pLayerCfg.Backcolor.Red = 0;
        //
        //    /* Configure blending factors */
        //    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
        //    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
        //
        //    /* Configure the number of lines and number of pixels per line */
        //    pLayerCfg.ImageWidth = 480;
        //    pLayerCfg.ImageHeight = 272;
    }

    void set_back_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
        m_layer.Backcolor.Red = red;
        m_layer.Backcolor.Green = green;
        m_layer.Backcolor.Blue = blue;
        m_layer.Alpha0 = alpha; /* fully transparent */
    }

    void set_pixel_format(int fmt) {
        m_layer.PixelFormat = fmt;
    }

    void set_window(int x0, int y0, int x1, int y1) {
        m_layer.WindowX0 = x0;
        m_layer.WindowX1 = x1;
        m_layer.WindowY0 = y0;
        m_layer.WindowY1 = y1;
    }

    void set_blending(int factor1, int factor2) {
        m_layer.BlendingFactor1 = factor1;
        m_layer.BlendingFactor2 = factor2;
    }

    void set_framebuffer(void* data) {
        m_layer.FBStartAdress = reinterpret_cast<uintptr_t>(data);
    }

    void set_alpha(uint8_t val) {
        m_layer.Alpha = val;
    }

    void set_dimensions(int width, int height) {
        m_layer.ImageWidth = width;
        m_layer.ImageHeight = height;
    }


    const LTDC_LayerCfgTypeDef* native_handle() const {
        return &m_layer;
    }

private:
    LTDC_LayerCfgTypeDef m_layer{};
};

class ltdc {
public:
    ltdc(const ltdc_layer& layer);

private:
    void gpio_config();
    void config();

    LTDC_HandleTypeDef m_ltdc;
};
} // namespace tos::stm32::f7