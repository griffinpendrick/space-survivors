#include "raylib.h"
#include "raymath.h"
#include "common.h"

bool32 DoButton(const char* Text, int32 X, int32 Y, uint32 FontSize)
{
    Vector2 MousePosition = GetMousePosition();
    Rectangle ButtonBounds = {(f32)X, (f32)Y, (f32)MeasureText(Text, FontSize), (f32)FontSize};

    bool32 IsHovering = CheckCollisionPointRec(MousePosition, ButtonBounds);
    Color TextColor = IsHovering ? YELLOW : WHITE;

    DrawText(Text, X, Y, FontSize, TextColor);

    if(IsHovering && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        PlaySound(ButtonClickSound);
        return(true);
    }

    return(false);
}