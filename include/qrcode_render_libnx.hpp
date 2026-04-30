#pragma once
#include <switch.h>
#include <string>
#include "qrcodegen.hpp"

void drawQRCodeToFramebuffer(const std::string& text, int scale = 4);
