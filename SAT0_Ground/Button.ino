/**
    @file Button.ino
    @brief Buttons listener for SATLLA0 GS.

    This file contains a button listener for the SATLLA0 GS.

    Copyright (C) 2023 @author Rony Ronen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* ============ */
/* Setup        */
/* ============ */

void button_setup() 
{
  PRINTLN("Func: button_setup()");
  pinMode(BTN_PRG_PIN, INPUT);

  button_pressed_flag = false;
  attachInterrupt(digitalPinToInterrupt(BTN_PRG_PIN), button_pressed, FALLING);
}

void button_pressed()
{
  PRINTLN("Func: button_pressed()");
  button_pressed_flag = true;
  detachInterrupt(digitalPinToInterrupt(BTN_PRG_PIN));
}


