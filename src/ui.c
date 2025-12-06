bool32 DoButton(const char* Text, int32 X, int32 Y, int32 FontSize)
{
    Rectangle ButtonBounds = {(f32)X, (f32)Y, (f32)MeasureText(Text, FontSize), (f32)FontSize};

    bool32 IsHovering = CheckCollisionPointRec(GetMousePosition(), ButtonBounds);
    Color TextColor = IsHovering ? YELLOW : WHITE;

    DrawText(Text, X, Y, FontSize, TextColor);

    if(IsHovering && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        PlaySound(ButtonClickSound);
        return(true);
    }

    return(false);
}

bool32 DoBoxedButton(const char* Text, int32 X, int32 Y, int32 W, int32 H, int32 FontSize)
{
    Rectangle ButtonBounds = {(f32)X, (f32)Y, (f32)W, (f32)H};
    bool32 IsHovering = CheckCollisionPointRec(GetMousePosition(), ButtonBounds);

    Color TextColor = IsHovering ? YELLOW : WHITE;

    int32 TextW = MeasureText(Text, FontSize);
    int32 TextX = X + (W / 2 - TextW / 2);
    int32 TextY = Y + (H / 2 - FontSize / 2);

    DrawText(Text, TextX, TextY, FontSize, TextColor);

    if(IsHovering && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        PlaySound(ButtonClickSound);
        return(true);
    }

    return(false);
}