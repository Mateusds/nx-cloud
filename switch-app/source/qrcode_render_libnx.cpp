#include "qrcode_render_libnx.hpp"
#include <switch.h>
#include <string>
#include <qrcodegen.hpp>

using namespace qrcodegen;

void drawPixel(u32* framebuf, u32 x, u32 y, u32 width, u32 height, u32 color) {
    if (x >= width || y >= height) return;
    framebuf[y * width + x] = color;
}

void drawQRCodeToFramebuffer(const std::string& text, int scale) {
    // Gera QR code
    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);
    
    int size = qr.getSize();
    int margin = 2;
    int qrSize = (size + margin * 2) * scale;
    
    // Obtém framebuffer
    u32* framebuf = (u32*)gfxGetFramebuffer(NULL, NULL);
    u32 width, height;
    gfxGetFramebufferResolution(&width, &height);
    
    // Cores
    u32 white = 0xFFFFFFFF;
    u32 black = 0xFF000000;
    
    // Calcula posição centralizada
    int startX = (width - qrSize) / 2;
    int startY = (height - qrSize) / 2;
    
    // Limpa área do QR code com branco
    for (int y = startY; y < startY + qrSize && y < height; y++) {
        for (int x = startX; x < startX + qrSize && x < width; x++) {
            drawPixel(framebuf, x, y, width, height, white);
        }
    }
    
    // Renderiza módulos do QR code
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qr.getModule(x, y)) {
                // Módulo preto
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = startX + (x + margin) * scale + sx;
                        int py = startY + (y + margin) * scale + sy;
                        if (px < width && py < height) {
                            drawPixel(framebuf, px, py, width, height, black);
                        }
                    }
                }
            }
        }
    }
    
    // Atualiza a tela
    gfxFlushBuffers();
}
