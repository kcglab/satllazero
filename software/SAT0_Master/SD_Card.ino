/**
    @file SD_Card.ino
    @brief IMU utils functions for SATLLA0.

    This file contains the utilities to maintain the SD card for the SATLLA0.
    It include the functions to write and read from the SD card.
    Using a 3rd library SdFat.

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

#if SD_ENABLE

#include "SdFat.h"
#include "sdios.h"

#define SD_FAT_TYPE 0

// File system object
SdFat sd;
SdFile dir;
// Use for file creation in folders.
SdFile file;

// For the GPS log file
SdFile sat_log_file;

char seqs_g[5] = {0};
char file_idx_g[4] = {0};

/* ============ */
/* Setup        */
/* ============ */

void SD_Card_setup()
{
    PRINTLN("Func: SD_Card_setup()");
    /* SD card */
    PRINTLN("Initializing SD card...");
    // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
    // Note that even if it's not used as the CS pin, the hardware SS pin
    // (10 on most Arduino boards, 53 on the Mega) must be left as an output
    // or the SD library functions will not work.
    // pinMode(10, OUTPUT);

    // if (!SD.begin(BUILTIN_SDCARD)) {
    //   PRINTLN("SD initialization failed!");
    // }

    // Initialize at the highest speed supported by the board that is
    // not over 50 MHz. Try a lower speed if SPI errors occur.
    // if (!SD.begin(chipSelect, SD_SCK_MHZ(50)))
    // if (!sd.begin())
    // {
    //     //sd.initErrorHalt();
    //     PRINTLN("SD initialization failed!");
    // }
    uint8_t counter = 0;
    bool state = sd.begin(SdioConfig(SHARED_SPI));
    while (!state && counter < SD_INIT_TRIES)
    {
        PRINT("SD initialization failed: \t");
        PRINTLN(counter);
        counter++;
        delay(TENTH_SEC);
        state = sd.begin(SdioConfig(SHARED_SPI));
    }

    if (state)
    {
        PRINTLN("SD initialization Succeed");
        sdcard_is_on = millis();
        // make sd the current volume.
        sd.chvol();
    }
    else
    {
        sdcard_is_on = 0;
    }

    make_folder(OUTBOX_FOLDER);
    make_folder(SENT_FOLDER);
    make_folder(RPI_FOLDER);

    sd_ls(&sd_data_g);
}

/* ============ */
/* Wipe SD      */
/* ============ */

// Use wipe() for no dot progress indicator.
void wipe_sd_card()
{
    PRINTLN("Func: wipe_sd_card()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    if (!sd.format(0)) // quick
    {
        PRINTLN("Error wipe the SD card");
    }
    else
    {
        PRINTLN("Wipe SD card succeed!");
    }
}

/* ============ */
/* Initial Boot */
/* ============ */

void initial_boot_completed_sd()
{
    PRINTLN("Func: initial_boot_completed_sd()");
    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    // if file exist then initial boot completed. No need to wait 30 min anymore.
    if (!sd.exists(INITIAL_BOOT_FILE))
    {
        // PRINTLN("SEQ_FILE: Write");
        file.open(INITIAL_BOOT_FILE, O_CREAT | O_WRITE);
        if (file.isOpen())
        {
            char time_stamp[10];
            sprintf(time_stamp, "%lu", millis());
            PRINTLN("lu");
            PRINTLN(time_stamp);
            file.write((uint8_t *)time_stamp, sizeof(time_stamp));
            file.close();
        }
        else
        {
            // if the file didn't open, print an error:
            PRINTLN("Error opening INITIAL_BOOT_FILE File");
        }
    }
}

bool is_initial_boot_completed_sd()
{
    PRINTLN("Func: is_initial_boot_completed_sd()");
    bool is_first_time = true;
    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    // if file exist then initial boot completed. No need to wait 30 min anymore.
    if (sd.exists(INITIAL_BOOT_FILE))
    {
        is_first_time = false;
    }
    return !is_first_time;
}

/* ============ */
/* LS           */
/* ============ */

int sd_ls_folder(char *dirname)
{
    PRINTLN("Func: sd_ls_folder()");

    uint16_t file_count = 0;
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    if (dir.open(dirname_g))
    {
        while (file.openNext(&dir, O_RDONLY) && file_count < FOLDER_MAX_FILES)
        {
            file.getName(dirname_g, sizeof(dirname_g));
            if (dirname_g[0] == '.' || file.isDir())
            {
                file.close();
                continue;
            }

#ifdef DEBUG
            file.printName(&Serial);
            file.printFileSize(&Serial);
            PRINTLN();
#endif
            file_count++;
            file.close();
        }
    }
    else
    {
        PRINT("Error: open dir failed!");
    }

    PRINT("Dirname:\t");
    PRINT(dirname);
    PRINT(", Count:\t");
    PRINTLN(file_count);

    return file_count;
}

uint16_t sd_ls(sd_data_t *sd_data)
{
    PRINTLN("Func: sd_ls()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    sd_data->sd_files = sd_ls_folder("/");
    sd_data->sd_outbox = sd_ls_folder(OUTBOX_FOLDER);
    sd_data->sd_sent = sd_ls_folder(SENT_FOLDER);
    sd_data->sd_rpi = sd_ls_folder(RPI_FOLDER);

    // uint32_t pos = 0;
    // uint8_t count_files = 0;
    // uint8_t count_folders = 0;
    // uint16_t file_count = 0;

    // memset(dirname_g, 0x00, DIRNAME_SIZE);

    //     while (file.openNext(&dir, O_RDONLY) && file_count < FOLDER_MAX_FILES * 5)
    //     {
    //         file_count++;
    // #ifdef DEBUG
    //         file.printName(&Serial);
    //         file.printFileSize(&Serial);
    // #endif
    //         file.getName(dirname_g, sizeof(dirname_g));
    //         if (dirname_g[0] == '.')
    //         {
    //             file.close();
    //             continue;
    //         }

    //         if (file.isDir())
    //         {
    //             // Indicate a directory.
    //             PRINT("Dirname:\t");
    //             PRINTLN(dirname_g);
    //             dir.open(dirname_g);
    //             sd_data->sd_outbox = sd_ls_folder(dirname_g);
    //             if (memcmp(dirname_g, OUTBOX_FOLDER, sizeof(OUTBOX_FOLDER)) == 0)
    //             {
    //                 pos = file.curPosition();
    //                 file.close();
    //                 sd_data->sd_outbox = sd_ls_folder(OUTBOX_FOLDER);
    //                 file.seekSet(pos);
    //             }
    //             else if (memcmp(dirname_g, SENT_FOLDER, sizeof(SENT_FOLDER)) == 0)
    //             {
    //                 pos = file.curPosition();
    //                 file.close();
    //                 sd_data->sd_sent = sd_ls_folder(SENT_FOLDER);
    //                 file.seekSet(pos);
    //             }
    //             else if (memcmp(dirname_g, RPI_FOLDER, sizeof(RPI_FOLDER)) == 0)
    //             {
    //                 pos = file.curPosition();
    //                 file.close();
    //                 sd_data->sd_rpi = sd_ls_folder(RPI_FOLDER);
    //                 file.seekSet(pos);
    //             }
    //             else
    //             {
    //                 count_folders++;
    //             }
    //         }
    //         else
    //         {
    //             // File on the root folder
    //             count_files++;
    //         }
    //         // PRINTLN();
    //         file.close();
    //     } // End while file entries available

    //     sd_data->sd_files = count_files;

    // sd.vwd()->rewind();
#ifdef PRINT_FUNC_DEBUG
    print_sd_data(sd_data);
#endif

    return sd_data->sd_files;
}

/* ================= */
/* Make Folder       */
/* ================= */

void make_folder(char *dirname)
{
    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    // Create a new folder.
    if (!sd.exists(dirname_g))
    {
        if (!sd.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed: ");
            PRINTLN(dirname_g);
        }
        else
        {
            PRINT("Info: Create dir succeeded: ");
            PRINTLN(dirname_g);
        }
    }
}

/* ================= */
/* Save Setting File */
/* ================= */

void write_setting_file(setting_data_t *setting_data)
{
    PRINTLN("Func: write_setting_file()");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    // SdFile setFile(SET_FILE, O_CREAT | O_WRITE);
    file.open(SET_FILE, O_CREAT | O_WRITE);
    if (file.isOpen())
    {
        file.write(setting_data->packet, SETTING_PACKET_LENGTH);
        file.close();
    }
    else
    {
        // if the file didn't open, print an error:
        PRINTLN("Error opening SET_FILE File");
    }
}

/* ================= */
/* Read Setting File */
/* ================= */

bool read_setting_file(setting_data_t *setting_data)
{
    PRINTLN("Func: read_setting_file()");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    bool ret_value = false;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    if (sd.exists(SET_FILE))
    {
        // PRINTLN("SET_FILE: Read");
        file.open(SET_FILE, O_READ);
        if (!file.isOpen())
        {
            PRINTLN("Error: SETTING file did not opened!");
        }
        else
        {
            // read from the file until there's nothing else in it:
            file.read(setting_data, SETTING_PACKET_LENGTH);
            file.close();

            for (uint8_t i = 0; i < SETTING_PACKET_LENGTH; i++)
            {
                PRINT(setting_data->packet[i], HEX);
                if ((i + 1) % 40 == 0)
                {
                    PRINTLN();
                }
            }
            PRINTLN();
            ret_value = true;
        }
    }
    else
    {
        PRINTLN("Error: File SETTING does not exist!");
    }
    return ret_value;
}

/* ============ */
/* Log 2 File   */
/* ============ */

void save_to_log(uint8_t *message, uint8_t length)
{
    PRINTLN("Func: save_to_log()");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    int sequence = get_log_index();

    // char sat_log_fileName[12 + 1] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s%05lu.%s", SAT_LOG_FILE, sequence, BIN_SUFFIX);
    PRINTLN(filename_g);
    // sat_log_file = SD.open(filename_g, FILE_WRITE);

    sat_log_file.open(filename_g, O_CREAT | O_APPEND | O_WRITE);
    uint32_t size = sat_log_file.fileSize();
    PRINT("sat_log_file Size: ");
    PRINTLN(size);

    if (size > LOG_SIZE)
    {
        sat_log_file.close();

        sequence += 1;
        set_log_index(sequence);
        delay(10);

        filename_g[FILENAME_SIZE] = {0};
        sprintf(filename_g, "%s%05lu.%s", SAT_LOG_FILE, sequence, BIN_SUFFIX);
        PRINT("Info: Log file name has been changed:'t");
        PRINTLN(filename_g);
        // sat_log_file = SD.open(filename_g, FILE_WRITE);
        sat_log_file.open(filename_g, O_CREAT | O_WRITE | O_APPEND);
    }

    // if the file opened okay, write to it:
    if (sat_log_file.isOpen())
    {
        PRINTLN("Info: Writing to sat_log_file: ");
        // Write to SD
        for (uint8_t i = 0; i < length; i++)
        {
            sat_log_file.write(message[i]);
        }
        print_buffer(message, length);
        sat_log_file.println();
        // close the file:
        sat_log_file.close();

        PRINTLN(". Done");
    }
    else
    {
        // if the file didn't open, print an error:
        PRINTLN("Error: Failed to open sat_log_file.");
    }
    PRINTLN("Func: save_to_log(): End");
}

/* =============== */
/* Read Log 2 File */
/* =============== */

uint8_t read_line_from_log_file(uint16_t line_number)
{
    PRINTLN("Func: read_line_from_log_file()");
    PRINT("Line Number:\t");
    PRINTLN(line_number);

    unsigned long sequence = 0;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    if (sd.exists(SEQ_FILE))
    {
        file.open(SEQ_FILE, O_READ);
        if (!file.isOpen())
        {
            PRINT("Error: Open SEQ file failed:");
            PRINTLN(SEQ_FILE);
        }
        else
        {
            // read from the file until there's nothing else in it:
            while (file.available())
            {
                // char seqs[5];
                file.read(&seqs_g, sizeof(seqs_g));
                sequence = atol(seqs_g);
            }
            // close the file:
            file.close();
        }
    }

    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s%05lu.%s", SAT_LOG_FILE, sequence, BIN_SUFFIX);
    PRINTLN(filename_g);

    int8_t input_char = 0;
    uint8_t idx = 0;
    uint32_t size_idx = 0;
    uint16_t lines = 0;
    bool done_reading = false;
    memset(buffer_g, 0x00, BUFFER_SIZE_MAX);
    // if the file opened okay, write to it:
    if (sat_log_file.open(filename_g, O_READ))
    {
        uint32_t size = sat_log_file.fileSize();
        PRINT("File size:\t");
        PRINTLN(size);
        sat_log_file.read(&input_char, 1); // get one byte from file
        size_idx++;
        while (input_char != -1 && size_idx < size) // if char not  eol
        {
            if (input_char == '\r' && size_idx < size)
            {
                sat_log_file.read(&input_char, 1); // get one byte from file
                size_idx++;
                if (input_char == '\n')
                {
                    // end of line
                    if (lines == line_number)
                    {
                        done_reading = true;
                        break;
                    }
                    else
                    {
                        lines++;
                        // PRINT("Lines:\t");
                        // PRINTLN(lines);
                        idx = 0;
                    }
                }
                else
                {
                    buffer_g[idx++] = input_char; // store it
                }
            }
            else
            {
                buffer_g[idx++] = input_char; // store it
            }
            sat_log_file.read(&input_char, 1); // get one byte from file
            size_idx++;
        }
        sat_log_file.close();

        if (done_reading)
        {
            PRINTLN("Info: Read from sat_log_file: ");
            // Write to SD
            print_buffer(buffer_g, idx);
            PRINTLN();
        }
    }
    else
    {
        // if the file didn't open, print an error:
        PRINTLN("Error: Failed to open sat_log_file.");
    }

    return idx;
}

/* ================= */
/* Read META         */
/* ================= */

bool read_meta_file(uint16_t mission)
{
    PRINTLN("Func: read_meta_file()");
    bool ret_value = false;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    if (!sd.exists(dirname_g))
    {
        PRINT("Error: dirnamr not exist:\t");
        PRINTLN(dirname_g);
    }
    else
    {
        filename_g[FILENAME_SIZE] = {0};
        sprintf(filename_g, "%s/%04x.BIN", dirname_g, mission);
        PRINT("File:\t");
        PRINTLN(filename_g);
        if (!file.open(filename_g, O_READ))
        {
            PRINT("Error: Read file failed:\t");
            PRINTLN(filename_g);
            // try under sent
            memset(dirname_g, 0x00, DIRNAME_SIZE);
            sprintf(dirname_g, "%s", SENT_FOLDER);
            PRINT("Dirname:\t");
            PRINTLN(dirname_g);

            filename_g[FILENAME_SIZE] = {0};
            sprintf(filename_g, "%s/%04x.BIN", dirname_g, mission);
            PRINT("File:\t");
            PRINTLN(filename_g);

            if (!file.open(filename_g, O_READ))
            {
                PRINT("Error: Read file failed:\t");
                PRINTLN(filename_g);
                return ret_value;
            }
        }

        uint32_t file_size = file.fileSize();
        file.read(&ld_meta_data_g, file_size);
        file.close();

        ret_value = true;

        PRINT("file_size:\t");
        PRINTLN(file_size);
        PRINTLN("Data:");
        print_buffer(ld_meta_data_g.packet, file_size);
        PRINTLN();
    }

    return ret_value;
}

/* ================= */
/* Save Meta File   */
/* ================= */

uint8_t save_meta_file(uint8_t *data, int data_len, uint16_t mission)
{
    PRINTLN("Func: save_meta_file()");
    uint8_t error = 1;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    // Save under MISSION
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    // If not exist, create a new folder.
    if (!sd.exists(dirname_g))
    {
        if (!sd.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
        }
    }

    // char fileName[FILENAME_SIZE] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x.BIN", dirname_g, mission);
    // Create a file in Folder1 using a path.
    if (!file.open(filename_g, O_CREAT | O_APPEND | O_WRITE))
    {
        PRINT("Error: create file failed:");
        PRINTLN(filename_g);
    }
    else
    {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        // if the file opened okay, write to it:
        PRINT("Writing to:\t");
        PRINT(filename_g);
        PRINT(", size: ");
        PRINT(data_len);
        // Write to SD
        for (int j = 0; j < data_len; j++)
        {
            file.write(data[j]);
        }
        file.close();
        error = 0;
        PRINTLN(", done.");
    }
    PRINTLN();
    return error;
}

/* ================= */
/* Save Large File   */
/* ================= */

uint8_t save_large_file(uint8_t *data, int data_len, uint16_t mission, uint16_t file_index)
{
    PRINTLN("Func: save_large_file()");

    uint8_t error = 1; // if done, error = 0;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    // Save under MISSION
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s/%04x", RPI_FOLDER, mission);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    // If not exist, create a new folder.
    if (!sd.exists(dirname_g))
    {
        if (!sd.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
        }
    }

    // char fileName[FILENAME_SIZE] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x.BIN", dirname_g, file_index);
    // Create a file in Folder1 using a path.
    if (!file.open(filename_g, O_CREAT | O_APPEND | O_WRITE))
    {
        PRINT("Error: create file failed:");
        PRINTLN(filename_g);
    }
    else
    {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        // if the file opened okay, write to it:
        PRINT("Writing to:\t");
        PRINT(filename_g);
        PRINT(", size: ");
        PRINT(data_len);
        // Write to SD
        for (int j = 0; j < data_len; j++)
        {
            // PRINTLN(data[j], HEX);
            file.write(data[j]);
        }
        file.close();
        delay(TENTH_SEC);
        error = 0;

        PRINTLN(", done.");
    }
    PRINTLN();
    return error;
}

/* ================= */
/* Read Large File   */
/* ================= */

uint32_t read_large_file(uint8_t *data, uint16_t mission, uint16_t file_index)
{
    PRINTLN("Func: read_large_file()");
    uint32_t file_size = 0;
    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    // Save under RPI
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", RPI_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    // if folder doesn't exist then return
    if (!sd.exists(dirname_g))
    {
        PRINT("Error: Folder not exist:");
        PRINTLN(dirname_g);
        return 0L;
    }

    // char fileName[FILENAME_SIZE] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x/%04x.BIN", dirname_g, mission, file_index);
    // Create a file in Folder1 using a path.
    if (!file.open(filename_g, O_READ))
    {
        PRINT("Error: open file failed:");
        PRINTLN(filename_g);
    }
    else
    {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        // if the file opened okay, write to it:
        PRINT("Reading file:\t");
        PRINT(filename_g);
        // Read from SD
        file_size = file.fileSize();
        file.read(data, file_size);
        file.close();
        PRINTLN(" done.");
    }
    // PRINTLN("Func: read_large_file(): End");
    // PRINT("file_size:\t");
    // PRINTLN(file_size);

    return file_size;
}

/* ================= */
/* Send Folder       */
/* ================= */

bool send_meta_files(char *dirname)
{
    PRINTLN("Func: send_meta_files()");
    bool ret_value = false;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);
    if (!sd.chdir(dirname_g))
    {
        PRINT("Error chdir:");
        PRINTLN(dirname_g);
    }
    else
    {
        filename_g[FILENAME_SIZE] = {0};
        uint8_t file_count = 0;
        while (file.openNext(&dir, O_RDONLY) && file_count < FOLDER_MAX_FILES)
        {
            file.getName(filename_g, sizeof(filename_g));
            PRINT("File:\t");
            PRINTLN(filename_g);
            file_count++;

            // reset global buffer
            memset(lrg_buffer_g, 0x00, LRG_BUFFER_SIZE_MAX);
            uint32_t fileSize = file.fileSize();
            PRINT("fileSize:\t");
            PRINTLN(fileSize);

            file.read(&lrg_buffer_g, fileSize);
            file.close();

            memset(ld_meta_data_g.packet, 0x00, LD_META_PACKET_LENGTH);
            ld_meta_data_g.ld_type = ld_type_meta;
            ld_meta_data_g.ld_size = fileSize;
            memcpy(ld_meta_data_g.ld_payload, lrg_buffer_g, fileSize);

            PRINT("ld_type:\t");
            PRINTLN(ld_meta_data_g.ld_type);
            PRINT("ld_size:\t");
            PRINTLN(ld_meta_data_g.ld_size);
            PRINTLN("Payload:");
            print_buffer(ld_meta_data_g.packet, LD_META_HEADER_LENGTH + fileSize);
            // for (uint8_t j = 0; j < LD_META_HEADER_LENGTH + fileSize; j++)
            // {
            //     if (ld_meta_data_g.packet[j] < 16)
            //     {
            //         PRINT(0);
            //     }
            //     PRINT(ld_meta_data_g.packet[j], HEX);
            //     if (!((j + 1) % 16))
            //     {
            //         PRINTLN();
            //     }
            //     else
            //     {
            //         PRINT(" ");
            //     }
            // }
            // PRINTLN();

            // Put file into Buffer
            if (!LDMETATXbuffer.isFull())
            {
                LDMETATXbuffer.add(ld_meta_data_g);
                PRINT("LDMETATXbuffer Element:\t");
                PRINTLN(LDMETATXbuffer.numElements());
            }
        }
        if (!LDMETATXbuffer.isEmpty())
        {
            ret_value = true;
        }
    }

    return ret_value;
}

/* ================= */
/* Clear Outbox      */
/* ================= */

void clear_outbox()
{
    PRINTLN("Func: clear_outbox()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", SENT_FOLDER);
    PRINTLN(dirname_g);

    // Create a new folder.
    if (!sd.exists(dirname_g))
    {
        if (!sd.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
        }
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);
    if (!sd.chdir(dirname_g))
    {
        PRINT("Error chdir:");
        PRINTLN(dirname_g);
    }
    else
    {
        uint8_t file_count = 0;
        while (file.openNext(&dir, O_WRITE) && file_count < FOLDER_MAX_FILES)
        {
            memset(filename_g, 0x00, FILENAME_SIZE);
            file.getName(filename_g, sizeof(filename_g));
            PRINT("From:\t");
            PRINTLN(filename_g);

            sd.chdir();
            memset(new_filename_g, 0x00, FILENAME_SIZE);
            sprintf(new_filename_g, "%s/%s", SENT_FOLDER, filename_g);
            PRINT("To:\t");
            PRINTLN(new_filename_g);

            if (!file.rename(new_filename_g))
            {
                PRINT("Error: rename file failed:\t");
                PRINTLN(filename_g);
                break;
            }
            else
            {
                PRINT("Info: rename file succeed:\t");
                PRINTLN(filename_g);
            }
            file.close();

            sd.chdir();
            if (!sd.chdir(dirname_g))
            {
                PRINT("Error chdir:");
                PRINTLN(dirname_g);
                break;
            }
        }
    }
}

/* ================= */
/* Delete Folder     */
/* ================= */

void delete_folder(char *dirname)
{
    PRINTLN("Func: delete_folder()");
    bool ret_value = false;

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    if (!dir.open("/"))
    {
        PRINT("Error: dir.open() failed:");
        PRINTLN(dirname_g);
    }
    else
    {
        uint16_t file_count = 0;
        while (file.openNext(&dir, O_WRITE) && file_count < FOLDER_MAX_FILES * 5)
        {
            file_count++;
            memset(filename_g, 0x00, FILENAME_SIZE);
            file.getName(filename_g, sizeof(filename_g));
            if (!file.remove())
            {
                PRINT("Error: Delete file:\t");
            }
            else
            {
                PRINT("File deleted:\t");
            }
            PRINTLN(filename_g);
        }
    }
}

/* ================ */
/* Days Operate     */
/* ================ */

bool set_operation_time_file(uint16_t time)
{
    PRINTLN("Func: set_operation_time_file()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    bool ret_value = false;
    file.open(OPERATE_FILE, O_CREAT | O_WRITE);
    if (!file.isOpen())
    {
        PRINTLN("Error Open: OPERATE_FILE file");
    }
    else
    {
        file.write(&time, sizeof(uint16_t));
        file.close();
        ret_value = true;
    }

    return ret_value;
}

uint16_t get_operation_time_file()
{
    PRINTLN("Func: get_operation_time_file()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    uint16_t time = 0;
    if (sd.exists(OPERATE_FILE))
    {
        file.open(OPERATE_FILE, O_READ);
        if (!file.isOpen())
        {
            PRINTLN("Error Open: OPERATE_FILE file");
        }
        else
        {
            file.read(&time, sizeof(uint16_t));
            // close the file:
            file.close();
        }
    }
    return time;
}

uint8_t increase_operation_half_days_file()
{
    PRINTLN("Func: increase_operation_half_days_file()");
    uint8_t error = 1;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    uint16_t time = 0;
    file.open(OPERATE_FILE, O_CREAT | O_WRITE);
    if (!file.isOpen())
    {
        PRINTLN("Error Open: OPERATE_FILE file");
    }
    else
    {
        file.read(&time, sizeof(uint16_t));
        time = time + 1;
        file.write(&time, sizeof(uint16_t));
        file.close();
        error = 0;
    }

    return error;
}

/* ================ */
/* Mission Index    */
/* ================ */

int get_misson_index()
{
    PRINTLN("Func: get_misson_index()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    int index = 0;
    if (sd.exists(IDX_FILE))
    {
        file.open(IDX_FILE, O_READ);
        if (!file.isOpen())
        {
            PRINTLN("Error: open IDX_FILE file failed");
        }
        else
        {
            file.read(&index, sizeof(int));
            // close the file:
            file.close();
        }
    }

    return index;
}

uint8_t set_misson_index(int index)
{
    PRINTLN("Func: set_misson_index()");

    uint8_t error = 1; // if done, error = 0;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    file.open(IDX_FILE, O_CREAT | O_WRITE);
    if (!file.isOpen())
    {
        PRINTLN("Error: open IDX_FILE file failed");
    }
    else
    {
        file.write(&index, sizeof(int));
        file.close();
        error = 0;
    }

    return error;
}

/* ================ */
/* Log Idx          */
/* ================ */

int get_log_index()
{
    PRINTLN("Func: get_log_index()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    int log_index = 0;
    if (sd.exists(SEQ_FILE))
    {
        file.open(SEQ_FILE, O_READ);
        if (!file.isOpen())
        {
            PRINTLN("Error Open: SEQ_FILE file");
        }
        else
        {
            file.read(&log_index, sizeof(int));
            // close the file:
            file.close();
        }
    }
    else
    {
        set_log_index(log_index);
    }

    PRINT("log_index:\t");
    PRINTLN(log_index);

    return log_index;
}

void set_log_index(int log_index)
{
    PRINTLN("Func: set_log_index()");
    PRINT("log_index:\t");
    PRINTLN(log_index);

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    file.open(SEQ_FILE, O_CREAT | O_WRITE);
    if (!file.isOpen())
    {
        PRINTLN("Error Open: SEQ_FILE file");
    }
    else
    {
        file.write(&log_index, sizeof(int));
        file.close();
    }
}

/* ================ */
/* Reset Index    */
/* ================ */

int get_reset_index()
{
    PRINTLN("Func: get_reset_index()");

    int index = 0;

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    else
    {
        if (sd.exists(RESET_FILE))
        {
            file.open(RESET_FILE, O_READ);
            if (!file.isOpen())
            {
                PRINTLN("Error: open RESET_FILE file failed");
            }
            else
            {
                file.read(&index, sizeof(int));
                // close the file:
                file.close();
                PRINT("Reset index:\t");
                PRINTLN(index);
            }
        }
        else
        {
            PRINTLN("Info: RESET_FILE doesn't exist");
        }
    }
    return index;
}

void set_reset_index(int index)
{
    PRINTLN("Func: set_reset_index()");

    // Open root directory
    if (!dir.open("/"))
    {
        PRINTLN("dir.open() failed");
    }
    else
    {
        file.open(RESET_FILE, O_CREAT | O_WRITE);
        if (!file.isOpen())
        {
            PRINTLN("Error: open RESET_FILE file failed");
        }
        else
        {
            file.write(&index, sizeof(int));
            file.close();
        }
    }
}

void inc_reset_index()
{
    PRINTLN("Func: inc_reset_index()");

    // read index
    int index = get_reset_index();
    // increament
    index++;
    // save
    set_reset_index(index);

    PRINT("Reset index:\t");
    PRINTLN(index);
}

#endif