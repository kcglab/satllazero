/**
    @file Serial_Event.ino
    @brief Serial Event functions for SATLLA0 GS.

    This file contains functionality to maintain serial events via PC communication for the SATLLA0 GS.

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
/* Serial Event */
/* ============ */
#define MAX_BUFFER_SIZE 1024  // Max buffer size of input
char ser_buffer_g[MAX_BUFFER_SIZE];

void serial_event_setup() {
    PRINTLN("***********************");
    PRINTLN("Settings and Commands:");
    PRINTLN("Send HEX Command: ");
    PRINTLN("r: Receive, s: S-BAND, u: UHF, p: Print");
    PRINTLN("{cmd}{dest}{code}{payload}");
    PRINTLN("***********************");
}

void serial_event() {
  PRINTLN("Func: serial_event()");

  int num_bytes = Serial.available();

  char c = 0;
  PRINT("num_bytes:\t");
  PRINTLN(num_bytes);
  if (num_bytes > MAX_BUFFER_SIZE) {
    num_bytes = MAX_BUFFER_SIZE;
  }

  if (num_bytes > 0) {
    // PRINT("num_bytes:\t");
    // PRINTLN(num_bytes);
    c = (char)Serial.read();
    PRINT("Command:\t");
    PRINTLN(c);
    
    switch (c) {
      case '\n':
      case '\r':
      case '\r\n':
        {
          // PRINTLN("CR LF");
        }
        break;
      case 'H':
      case 'h':
        {
          serial_event_setup();
#if WIFI_ENABLE
          print_ip();
#endif
        }
        break;

      case 'M':
      case 'm':
        {
          c = (char)Serial.read();
          uint8_t mode = c = '0';
          Serial.flush();
        }
        break;
#if TRK_ENABLE
      case 'a':
        {
          String az_s = Serial.readString();
          int az = (int)atol(az_s.c_str());
          PRINT("az:\t");
          PRINTLN(az);
          az_write(az);
          Serial.flush();
        }
        break;

      case 'e':
        {
          String el_s = Serial.readString();
          int el = (int)atol(el_s.c_str());
          PRINT("el:\t");
          PRINTLN(el);
          el_write(el);
          Serial.flush();
        }
        break;

      case 'n':
        {
          find_north();
          Serial.flush();
        }
        break;
#endif
      case 'I':
      case 'i':
        {
#if EEPROM_ENABLE
          reset_settings();
#endif
          Serial.flush();
        }
        break;

      default:
        bool send_flag = false;

        delay(SEC_1);

        num_bytes = Serial.available() - 1;  // CR
        PRINT("num_bytes:\t");
        PRINTLN(num_bytes);

        if (num_bytes > 0) {
          parse_command(c, num_bytes);
        }

        Serial.flush();
        break;
    }
  }
}

void parse_command(char cmd, int num_bytes) {
  PRINTLN("Func: parse_command()");
  uint8_t val = 0;
  uint8_t idx = 0;
  uint8_t b = 0;
  char c[2];
  String hex;
  lorafreq_e freq = lora_none;

memset(ser_buffer_g, 0x00, MAX_BUFFER_SIZE);

  for (int i = 0; i < num_bytes; i += 2)  // avoid eol
  {
    c[0] = (char)Serial.read();
    c[1] = (char)Serial.read();
    if (c[0] == -1 || c[1] == -1) {
      PRINTLN("Error: Exit");
      return;
    }
    hex = String(c[0]) + String(c[1]);
    val = (int)strtol(hex.c_str(), NULL, 16);
    ser_buffer_g[idx++] = val;
  }

#ifdef DEBUG
  print_buffer((uint8_t *)ser_buffer_g, idx);
#endif

  if (cmd == 't') {
#if EEPROM_ENABLE
    PRINTLN("Settings:");
    print_settings();
    print_eeprom();
#endif
    if (idx >= 2) {
      uint8_t opt = (int)ser_buffer_g[0] + 87;
      PRINT("opt:\t");
      PRINTLN((char)opt);
#if EEPROM_ENABLE
    parse_setting(opt, ser_buffer_g + 1, idx - 1);
#endif
    }
    return;
  }

  memset(command_data_g.packet, 0x00, COMMAND_PACKET_LENGTH);
  uint8_t destination = ser_buffer_g[0];
  command_data_g.cmd_code = ser_buffer_g[1];
  command_data_g.cmd_type = cmd_type_param;
  command_data_g.cmd_size = idx - 2;
  memcpy(command_data_g.cmd_payload, ser_buffer_g + 2, idx - 2);
  // print_command(&command_data_g);

  if (cmd == 'p') {
    memcpy(lora_data_g.packet, ser_buffer_g, idx);
    print_message(lora_data_g.packet, LORA_HEADER_LENGTH + lora_data_g.msg_size, freq);

    if (lora_data_g.msg_type == type_command) {
      memcpy(command_data_g.packet, lora_data_g.msg_payload, lora_data_g.msg_size);
      parse_and_activate_command(&command_data_g, destination);
    }
  } else {
    if (cmd == 'r') {
      cmd_receive(&command_data_g, destination);
    } else {
      if (cmd == 's') {
        freq = lora_24;
      } else if (cmd == 'u') {
        freq = lora_433;
      }
      if (destination != 0x00) {
        cmd_send(&command_data_g, destination, freq);
      }
    }
  }
}