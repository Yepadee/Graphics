#pragma once
#include <DrawingWindow.h>

void antiAlias(const DrawingWindow& inputWindow, DrawingWindow& outputWindow)
{
    const int scale = 3;
    for (int i = 0; i < outputWindow.width; i++)
    {
        for (int j = 0; j < outputWindow.height; j++)
        {
            int inputX = scale * i + 1;
            int inputY = scale * j + 1;

            uint32_t topLeft = inputWindow.getPixelColour(inputX - 1, inputY - 1);
            uint32_t bottomLeft = inputWindow.getPixelColour(inputX - 1, inputY + 1);
            uint32_t topRight = inputWindow.getPixelColour(inputX + 1, inputY - 1);
            uint32_t bottomRight = inputWindow.getPixelColour(inputX + 1, inputY + 1);
            uint32_t center = inputWindow.getPixelColour(inputX, inputY);

            uint32_t average = (center * 2 + topLeft + bottomLeft + topRight + bottomRight) / 6;

            outputWindow.setPixelColour(i,j,average);
        }
    }
}