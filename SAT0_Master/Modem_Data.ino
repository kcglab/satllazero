/**
    @file Modem_Data.ino
    @brief Modem struct for SATLLA0.

    This file contains radio parameters for the SATLLA0.

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

/* ============= */
/* Modem         */
/* ============= */

void modem_lora_433_setup()
{
#if RF_433_ENABLE
    PRINTLN(F("Func: modem_lora_433_setup()"));

    modem_lora_433_g.modem_frequency = LORA_433_BAND;
    modem_lora_433_g.modem_offset = LORA_433_OFFSET;
    modem_lora_433_g.modem_bw = LORA_433_BW;             // the closest number to the BW
    modem_lora_433_g.modem_cr = LORA_433_CR;             // from 5 to 8
    modem_lora_433_g.modem_sf = LORA_433_SF;             // from 6 to 12
    modem_lora_433_g.modem_power = LORA_433_TX_RX_POWER; // 5~20
    modem_lora_433_g.modem_ldro = LORA_433_LDRO;         // 0x00 OFF, 0x01 ON, 0x02 AUTO
    modem_lora_433_g.modem_pl = LORA_433_PL;             // Preamble 8
    modem_lora_433_g.modem_sw = LORA_433_SW;             // Sync Word
    modem_lora_433_g.modem_crc = LORA_433_CRC;           // On/Off
#endif
}

void modem_fsk_433_setup()
{
#if RF_433_ENABLE
    PRINTLN(F("Func: modem_fsk_433_setup()"));

    modem_fsk_433_g.modem_frequency = LORA_433_BAND;
    modem_fsk_433_g.modem_offset = FSK_433_DEVIATION;
    modem_fsk_433_g.modem_bw = FSK_433_BW;             // the closest number to the BW
    modem_fsk_433_g.modem_cr = FSK_433_CR;             // from 5 to 8
    modem_fsk_433_g.modem_br = FSK_433_BR;             // bit rate, from  1.2 300.0
    modem_fsk_433_g.modem_power = FSK_433_TX_RX_POWER; // 5~20
    modem_fsk_433_g.modem_ds = FSK_433_DS;             // 0x00 OFF, 0x01 ON, 0x02 AUTO
    modem_fsk_433_g.modem_pl = FSK_433_PL;             // Preamble 8
    modem_fsk_433_g.modem_sw = FSK_433_SW;             // Sync Word
    modem_fsk_433_g.modem_crc = FSK_433_CRC;           // On/Off
#endif
}

void modem_lora_24_setup()
{
#if RF_24_ENABLE
    PRINTLN(F("Func: modem_lora_24_setup()"));

    modem_lora_24_g.modem_frequency = LORA_24_BAND;
    modem_lora_24_g.modem_offset = LORA_24_OFFSET;
    modem_lora_24_g.modem_bw = LORA_24_BW;             // the closest number to the BW
    modem_lora_24_g.modem_cr = LORA_24_CR;             // from 5 to 8
    modem_lora_24_g.modem_sf = LORA_24_SF;             // from 6 to 12
    modem_lora_24_g.modem_power = LORA_24_TX_RX_POWER; // 5~20
    modem_lora_24_g.modem_ldro = LORA_24_LDRO;         // 0x00 OFF, 0x01 ON, 0x02 AUTO
    modem_lora_24_g.modem_pl = LORA_24_PL;             // Preamble 8
    modem_lora_24_g.modem_sw = LORA_24_SW;             // Sync Word
    modem_lora_24_g.modem_crc = LORA_24_CRC;           // On/Off
#endif
}

/* ============== */
/* Print Battery  */
/* ============== */

void print_modem(modem_data_t *modem_data)
{
    PRINTLN(F("Func: print_modem()"));

    PRINT(F("modem_bw:\t"));
    PRINTLN(modem_data->modem_bw);

    PRINT(F("modem_cr:\t"));
    PRINTLN(modem_data->modem_cr);

    PRINT(F("modem_sf:\t"));
    PRINTLN(modem_data->modem_sf);

    PRINT(F("modem_power:\t"));
    PRINTLN(modem_data->modem_power);

    PRINT(F("modem_ldro:\t"));
    PRINTLN(modem_data->modem_ldro);

    PRINT(F("modem_pl:\t"));
    PRINTLN(modem_data->modem_pl);

    PRINT(F("modem_crc:\t"));
    PRINTLN(modem_data->modem_crc);

    PRINT(F("modem_ds:\t"));
    PRINTLN(modem_data->modem_ds);

    PRINT(F("modem_sw:\t0x"));
    PRINTLN(modem_data->modem_sw, HEX);

    PRINT(F("modem_freq:\t"));
    PRINTLN(modem_data->modem_frequency);

    PRINT(F("modem_offset:\t"));
    PRINTLN(modem_data->modem_offset);

    PRINT(F("modem_br:\t"));
    PRINTLN(modem_data->modem_br);
    PRINTLN();
}