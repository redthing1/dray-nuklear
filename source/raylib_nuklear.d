/**********************************************************************************************
*
*   raylib-nuklear v5.0.0 - Nuklear GUI for Raylib.
*
*   FEATURES:
*       - Use the Nuklear immediate-mode graphical user interface in raylib.
*
*   DEPENDENCIES:
*       - raylib 4.5+ https://www.raylib.com/
*       - Nuklear https://github.com/Immediate-Mode-UI/Nuklear
*
*   LICENSE: zlib/libpng
*
*   raylib-nuklear is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2020 Rob Loach (@RobLoach)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

module raylib_nuklear;

public {
    static import raylib; // raylib
    static import nuklear; // nuklear gui
    static import cassowary; // constraint solver
}
import raylib;
import nuklear;

extern (C) @nogc nothrow:

nk_context* InitNuklear (int fontSize); // Initialize the Nuklear GUI context
nk_context* InitNuklearEx (Font font, float fontSize); // Initialize the Nuklear GUI context, with a custom font
void UpdateNuklear (nk_context* ctx); // Update the input state and internal components for Nuklear
void DrawNuklear (nk_context* ctx); // Render the Nuklear GUI on the screen
void UnloadNuklear (nk_context* ctx); // Deinitialize the Nuklear context
nk_color ColorToNuklear (Color color); // Convert a raylib Color to a Nuklear color object
nk_colorf ColorToNuklearF (Color color); // Convert a raylib Color to a Nuklear floating color
Color ColorFromNuklear (nk_color color); // Convert a Nuklear color to a raylib Color
Color ColorFromNuklearF (nk_colorf color); // Convert a Nuklear floating color to a raylib Color
Rectangle RectangleFromNuklear (nk_context* ctx, nk_rect_ rect); // Convert a Nuklear rectangle to a raylib Rectangle
nk_rect_ RectangleToNuklear (nk_context* ctx, Rectangle rect); // Convert a raylib Rectangle to a Nuklear Rectangle
nk_image_ TextureToNuklear (Texture tex); // Convert a raylib Texture to A Nuklear image
Texture TextureFromNuklear (nk_image_ img); // Convert a Nuklear image to a raylib Texture
nk_image_ LoadNuklearImage (const(char)* path); // Load a Nuklear image
void UnloadNuklearImage (nk_image_ img); // Unload a Nuklear image. And free its data
void CleanupNuklearImage (nk_image_ img); // Frees the data stored by the Nuklear image
void SetNuklearScaling (nk_context* ctx, float scaling); // Sets the scaling for the given Nuklear context
float GetNuklearScaling (nk_context* ctx); // Retrieves the scaling of the given Nuklear context

// RAYLIB_NUKLEAR_H