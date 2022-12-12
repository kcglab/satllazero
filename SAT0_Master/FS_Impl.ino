/**
    @file FS_Imple.ino
    @brief Flash memory function for SATLLA0.

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

#if TNSYFS_ENABLE

#include <LittleFS.h>

void fs_ls_dir(FS &fs);
uint16_t fs_ls_folder(File dir);

// This declares the LittleFS Media type and gives a text name to Identify in use
LittleFS_Program tnsyfs;

#define PROG_FLASH_SIZE (1024 * 960 * 8) // Specify size to use of onboard Teensy Program Flash chip. This creates a LittleFS drive in Teensy PCB FLash.

uint32_t diskSize;
uint32_t lastTime;

void tnsyfs_setup()
{
    PRINTLN("Func: tnsyfs_setup()");

    // see if you are able to create a RAM disk in the space you a lotted
    // buf = is the name of the array you created, sizedf(buf) is how large the
    // array is, in our case 390 * 1024.
    if (!tnsyfs.begin(PROG_FLASH_SIZE))
    {
        PRINTLN("LittleFS: Setup Failed!");
        flash_is_on = 0;
    }
    else
    {
        PRINT("LittleFS: Setup Successed: ");
        PRINT(PROG_FLASH_SIZE);
        PRINTLN(" bytes");
        flash_is_on = millis();


        fs_make_folder(OUTBOX_FOLDER);
        fs_make_folder(SENT_FOLDER);
        fs_make_folder(RPI_FOLDER);

        fs_ls_dir(tnsyfs);
    }
}

/* ============ */
/* Wipe FS      */
/* ============ */

// Use wipe() to format the FS
void wipe_fs()
{
    PRINTLN("Func: wipe_fs()");
    uint32_t timeMe;

    lastTime = micros();
    timeMe = micros();
    tnsyfs.lowLevelFormat('.');
    timeMe = micros() - timeMe;
    PRINT("Done Formatting Low Level: ");
    PRINTLN(timeMe);
}

/* ============ */
/* Erase FS      */
/* ============ */

// Use erase() to delete all files.
void erase_fs()
{
    PRINTLN("Func: erase_fs()");
    uint32_t timeMe;

    lastTime = micros();
    timeMe = micros();
    tnsyfs.quickFormat();
    timeMe = micros() - timeMe;
    PRINT("Done Quick Formatting: ");
    PRINTLN(timeMe);
}

/* ============ */
/* LS           */
/* ============ */

void fs_ls_dir(FS &fs)
{
    PRINTLN("Func: fs_ls_dir()");

    PRINT("Filesystem Size = ");
    PRINTLN(tnsyfs.totalSize());
    PRINT("Space Used = ");
    PRINTLN(tnsyfs.usedSize());

    fs_ls_folder(fs.open("/"));
}

uint16_t fs_ls_folder(char *dirname)
{
    PRINTLN("Func: fs_ls_folder(1)");
    File dir = tnsyfs.open(dirname);
    // Open root directory
    if (!dir)
    {
        PRINTLN("tnsyfs.open() failed");
        return 0;
    }

    return fs_ls_folder(dir);
}

uint16_t fs_ls_folder(File dir)
{
    PRINTLN("Func: fs_ls_folder(2)");
    PRINT("Dirname:\t");
    PRINTLN(dir.name());

    uint16_t file_count = 0;
    while (file_count < FOLDER_MAX_FILES)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            PRINTLN("** no more files **");
            break;
        }

        if (entry.isDirectory())
        {
            PRINT("Dirname:\t");
            PRINT(entry.name());
            PRINTLN("/");
            file_count += fs_ls_folder(entry);
        }
        else
        {
            // files have sizes, directories do not
            PRINT("Filename:\t");
            PRINT(entry.name());
            PRINT(", ");
            PRINTLN(entry.size(), DEC);
            file_count++;
        }
        entry.close();
    }
    PRINT("FS File Count:\t");
    PRINTLN(file_count);

    return file_count;
}

uint16_t fs_ls(sd_data_t *sd_data)
{
    PRINTLN("Func: fs_ls()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    uint32_t pos = 0;
    uint8_t count_files = 0;
    uint8_t count_folders = 0;
    uint16_t file_count = 0;

    memset(dirname_g, 0x00, DIRNAME_SIZE);

    File dir = tnsyfs.open("/");

    while (file_count < FOLDER_MAX_FILES)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // PRINTLN("** no more files **");
            break;
        }

        if (entry.isDirectory())
        {
            // Indicate a directory.
            PRINT("Dirname:\t");
            PRINTLN(entry.name());

            memcpy(dirname_g, entry.name(), 8);
            if (memcmp(dirname_g, OUTBOX_FOLDER, sizeof(OUTBOX_FOLDER)) == 0)
            {
                // entry.close();
                uint16_t fc = fs_ls_folder(OUTBOX_FOLDER);
                sd_data->sd_outbox = fc;
                file_count+=fc;
            }
            else if (memcmp(dirname_g, SENT_FOLDER, sizeof(SENT_FOLDER)) == 0)
            {
                // entry.close();
                uint16_t fc = fs_ls_folder(SENT_FOLDER);
                sd_data->sd_sent = fc;
                file_count+=fc;
            }
            else if (memcmp(dirname_g, RPI_FOLDER, sizeof(RPI_FOLDER)) == 0)
            {
                // entry.close();
                uint16_t fc = fs_ls_folder(RPI_FOLDER);
                sd_data->sd_rpi = fc;
                file_count+=fc;
            }
            else
            {
                count_folders++;
            }
        }
        else
        {
            PRINT("Filename:\t");
            PRINT(entry.name());
            PRINT(", ");
            PRINTLN(entry.size(), DEC);
            // File on the root folder
            count_files++;
            entry.close();
        }
        // PRINTLN();
        file_count++;
    } // End while file entries available

    sd_data->sd_files = count_files;

    // sd.vwd()->rewind();
#ifdef PRINT_FUNC_DEBUG
    print_sd_data(sd_data);
#endif
    return file_count;
}


/* ================= */
/* Make Folder       */
/* ================= */

void fs_make_folder(char *dirname)
{
    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    // Create a new folder.
    if (!tnsyfs.exists(dirname_g))
    {
        if (!tnsyfs.mkdir(dirname_g))
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

void fs_write_setting_file(setting_data_t *setting_data)
{
    PRINTLN("Func: fs_write_setting_file()");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    File file = tnsyfs.open(SET_FILE, FILE_WRITE);
    // if the file is available, write to it:
    if (file)
    {
        file.seek(0); // file_write includes append
        file.write(setting_data->packet, SETTING_PACKET_LENGTH);
        file.close();
#ifdef DEBUG
        print_buffer(setting_data->packet, SETTING_PACKET_LENGTH);
#endif
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

bool fs_read_setting_file(setting_data_t *setting_data)
{
    PRINTLN("Func: read_setting_file()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    bool ret_value = false;
    if (tnsyfs.exists(SET_FILE))
    {
        // PRINTLN("SET_FILE: Read");
        File file = tnsyfs.open(SET_FILE, FILE_READ);
        if (file)
        {
            // read from the file until there's nothing else in it:
            file.read(setting_data, SETTING_PACKET_LENGTH);
            file.close();
#ifdef DEBUG
            print_buffer(setting_data->packet, SETTING_PACKET_LENGTH);
#endif
            ret_value = true;
        }
        else
        {
            PRINTLN("Error: SETTING file did not opened!");
        }
    }
    else
    {
        PRINTLN("Error: File SETTING does not exist!");
    }
    return ret_value;
}

/* ================= */
/* Save Meta File   */
/* ================= */

uint8_t fs_save_meta_file(uint8_t *data, int data_len, uint16_t mission)
{
    PRINTLN("Func: fs_save_meta_file()");
    uint8_t error = 1;

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    // Save under MISSION
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    // If not exist, create a new folder.
    if (!tnsyfs.exists(dirname_g))
    {
        if (!tnsyfs.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
            return;
        }
    }

    // char fileName[FILENAME_SIZE] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x.BIN", dirname_g, mission);
    // Create a file in Folder1 using a path.
    File file = tnsyfs.open(filename_g, FILE_WRITE);
    if (file)
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
    else
    {
        PRINT("Error: create file failed:");
        PRINTLN(filename_g);
    }
    PRINTLN();
    return error;
}

/* ================= */
/* Read META         */
/* ================= */

bool fs_read_meta_file(uint16_t mission)
{
    PRINTLN("Func: fs_read_meta_file()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    bool ret_value = false;

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    if (tnsyfs.exists(dirname_g))
    {
        filename_g[FILENAME_SIZE] = {0};
        sprintf(filename_g, "%s/%04x.BIN", dirname_g, mission);
        PRINT("File:\t");
        PRINTLN(filename_g);

        File file = tnsyfs.open(filename_g, FILE_READ);
        if (!file)
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

            file = tnsyfs.open(filename_g, FILE_READ);
            if (!file)
            {
                PRINT("Error: Read file failed:\t");
                PRINTLN(filename_g);
                return ret_value;
            }
        }

        uint32_t file_size = file.size();
        file.read(&ld_meta_data_g, file_size);
        file.close();

        ret_value = true;

        PRINT("file_size:\t");
        PRINTLN(file_size);
        PRINTLN("Data:");
#ifdef DEBUG
        print_buffer(ld_meta_data_g.packet, file_size);
#endif
        PRINTLN();
    }
    else
    {
        PRINT("Error: dirnamr not exist:\t");
        PRINTLN(dirname_g);
    }

    return ret_value;
}

/* ================= */
/* Save Large File   */
/* ================= */

uint8_t fs_save_large_file(uint8_t *data, int data_len, uint16_t mission, uint16_t file_index)
{
    PRINTLN("Func: fs_save_large_file()");

    uint8_t error = 1; // if done, error = 0;

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    // Open RPI directory
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", RPI_FOLDER);

    // If not exist, create a new folder.
    if (!tnsyfs.exists(dirname_g))
    {
        sprintf(dirname_g, "%s", RPI_FOLDER);
        if (!tnsyfs.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
            return;
        }
    }

    // Open mission directory
    sprintf(dirname_g, "%s/%04x", RPI_FOLDER, mission);
    if (!tnsyfs.exists(dirname_g))
    {
        if (!tnsyfs.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:");
            PRINTLN(dirname_g);
        }
    }

    // Save in mission directory
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x/%04x.BIN", RPI_FOLDER, mission, file_index);
    File file = tnsyfs.open(filename_g, FILE_WRITE);
    if (file)
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
        delay(TENTH_SEC);
        error = 0;

        PRINTLN(", done.");
    }
    else
    {
        PRINT("Error: create file failed:");
        PRINTLN(filename_g);
    }
    PRINTLN();
    return error;
}

/* ================= */
/* Read Large File   */
/* ================= */

uint32_t fs_read_large_file(uint8_t *data, uint16_t mission, uint16_t file_index)
{
    PRINTLN("Func: fs_read_large_file()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    uint32_t file_size = 0;

    // Save under RPI
    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", RPI_FOLDER);
    PRINT("Dirname:\t");
    PRINTLN(dirname_g);

    // if folder doesn't exist then return
    if (!tnsyfs.exists(dirname_g))
    {
        PRINT("Error: Folder not exist:");
        PRINTLN(dirname_g);
        return 0L;
    }

    // char fileName[FILENAME_SIZE] = {0};
    filename_g[FILENAME_SIZE] = {0};
    sprintf(filename_g, "%s/%04x/%04x.BIN", dirname_g, mission, file_index);
    // Create a file in Folder1 using a path.
    File file = tnsyfs.open(filename_g, FILE_READ);
    if (file)
    {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        // if the file opened okay, write to it:
        PRINT("Reading file:\t");
        PRINT(filename_g);
        // Read from SD
        file_size = file.size();
        file.read(data, file_size);
        file.close();
        PRINTLN(" done.");
    }
    else
    {
        PRINT("Error: open file failed:");
        PRINTLN(filename_g);
    }
    // PRINTLN("Func: read_large_file(): End");
    // PRINT("file_size:\t");
    // PRINTLN(file_size);

    return file_size;
}

/* ================= */
/* Send Folder       */
/* ================= */

bool fs_send_meta_files(char *dirname){
    return fs_send_meta_files(dirname, TX_RBUFF_MAX_NUM_ELEMENTS);
}

// maxSize : keep x numElements
bool fs_send_meta_files(char *dirname, uint8_t maxSize)
{
    PRINTLN("Func: fs_send_meta_files()");
    bool ret_value = false;

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    File dir = tnsyfs.open(dirname_g);

    if (!dir)
    {
        PRINTLN("tnsyfs.open() failed. Folder:\t");
        PRINTLN(dirname_g);
    }
    else
    {
        filename_g[FILENAME_SIZE] = {0};
        uint8_t file_count = 0;

        while (file_count < FOLDER_MAX_FILES)
        {
            File entry = dir.openNextFile();
            if (!entry)
            {
                // PRINTLN("** no more files **");
                break;
            }

            memcpy(filename_g, entry.name(), sizeof(filename_g));
            PRINT("File:\t");
            PRINTLN(filename_g);
            file_count++;

            // reset global buffer
            memset(lrg_buffer_g, 0x00, LRG_BUFFER_SIZE_MAX);
            uint32_t fileSize = entry.size();

            PRINT("fileSize:\t");
            PRINTLN(fileSize);

            entry.read(&lrg_buffer_g, fileSize);
            entry.close();

            memset(ld_meta_data_g.packet, 0x00, LD_META_PACKET_LENGTH);
            ld_meta_data_g.ld_type = ld_type_meta;
            ld_meta_data_g.ld_size = fileSize;
            memcpy(ld_meta_data_g.ld_payload, lrg_buffer_g, fileSize);

            print_ld_meta_data(&ld_meta_data_g);

            // if reach full or max requested, keep the x last ones.
            if (LDMETATXbuffer.isFull() || LDMETATXbuffer.numElements() > maxSize)
            {
                ld_meta_data_t temp;
                LDMETATXbuffer.pull(&temp);
            }
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

void fs_clear_outbox()
{
    PRINTLN("Func: fs_clear_outbox()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", SENT_FOLDER);
    PRINTLN(dirname_g);

    // Create a new folder.
    if (!tnsyfs.exists(dirname_g))
    {
        if (!tnsyfs.mkdir(dirname_g))
        {
            PRINT("Error: Create dir failed:\t");
            PRINTLN(dirname_g);
            return;
        }
        else
        {
            PRINT("Info: Create dir succeed:\t");
            PRINTLN(dirname_g);
        }
    }

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", OUTBOX_FOLDER);

    File dir = tnsyfs.open(dirname_g);

    if (!tnsyfs.open(dirname_g))
    {
        PRINT("Error: chdir failed:\t");
        PRINTLN(dirname_g);
    }
    else
    {
        uint8_t file_count = 0;
        while (file_count < FOLDER_MAX_FILES)
        {
            File entry = dir.openNextFile();
            if (!entry)
            {
                PRINTLN("** no more files **");
                break;
            }

            memset(filename_g, 0x00, FILENAME_SIZE);
            memcpy(filename_g, entry.name(), sizeof(filename_g));
            uint32_t fileSize = entry.size();

            // reset global buffer
            memset(lrg_buffer_g, 0x00, LRG_BUFFER_SIZE_MAX);
            entry.read(&lrg_buffer_g, fileSize);
            entry.close();

            tnsyfs.open("/");
            sprintf(dirname_g, "%s", SENT_FOLDER);
            tnsyfs.open(dirname_g);

            File new_entry = tnsyfs.open(filename_g, FILE_WRITE);

            if (new_entry)
            {
                new_entry.write(&lrg_buffer_g, fileSize);
                new_entry.close();
                PRINT("Info: rename file succeed:\t");
                PRINTLN(filename_g);
            }
            else
            {
                PRINT("Error: rename file failed:\t");
                PRINTLN(filename_g);
                break;
            }

            tnsyfs.open("/");
            sprintf(dirname_g, "%s", OUTBOX_FOLDER);
            if (!tnsyfs.open(dirname_g))
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

void fs_delete_folder(char *dirname)
{
    PRINTLN("Func: fs_delete_folder()");
    bool ret_value = false;

    memset(dirname_g, 0x00, DIRNAME_SIZE);
    sprintf(dirname_g, "%s", dirname);

    File dir = tnsyfs.open(dirname_g);

    if (!tnsyfs.open(dirname_g))
    {
        PRINT("Error: chdir failed:\t");
        PRINTLN(dirname_g);
    }
    else
    {
        uint16_t file_count = 0;
        while (file_count < FOLDER_MAX_FILES)
        {
            File entry = dir.openNextFile();
            if (!entry)
            {
                // PRINTLN("** no more files **");
                break;
            }
            if (!entry.isDirectory())
            {
                file_count++;
                memcpy(filename_g, entry.name(), sizeof(filename_g));
                if (!tnsyfs.remove(filename_g))
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
}

/* ================ */
/* Mission Index    */
/* ================ */

int fs_get_misson_index()
{
    PRINTLN("Func: fs_get_misson_index()");

    int index = 0;
    if (tnsyfs.exists(IDX_FILE))
    {
        File file = tnsyfs.open(IDX_FILE, FILE_READ);
        if (file)
        {
            file.read(&index, sizeof(int));
            // close the file:
            file.close();
        }
        else
        {
            PRINTLN("Error: open IDX_FILE file failed");
        }
    }

    return index;
}

uint8_t fs_set_misson_index(int index)
{
    PRINTLN("Func: fs_set_misson_index()");

    uint8_t error = 1; // if done, error = 0;

    File file = tnsyfs.open(IDX_FILE, FILE_WRITE);
    if (file)
    {
        file.seek(0); // file_write includes append
        file.write(&index, sizeof(int));
        file.close();

        error = 0;
    }
    else
    {
        PRINTLN("Error: open IDX_FILE file failed");
    }

    return error;
}

/* ================ */
/* Log Idx          */
/* ================ */

int fs_get_log_index()
{
    PRINTLN("Func: fs_get_log_index()");

    int log_index = 0;
    if (tnsyfs.exists(SEQ_FILE))
    {
        File file = tnsyfs.open(SEQ_FILE, FILE_READ);
        if (file)
        {
            file.read(&log_index, sizeof(int));
            // close the file:
            file.close();
        }
        else
        {
            PRINTLN("Error Open: SEQ_FILE file");
        }
    }
    else
    {
        fs_set_log_index(log_index);
    }

    PRINT("log_index:\t");
    PRINTLN(log_index);

    return log_index;
}

void fs_set_log_index(int log_index)
{
    PRINTLN("Func: fs_set_log_index()");
    PRINT("log_index:\t");
    PRINTLN(log_index);

    File file = tnsyfs.open(SEQ_FILE, FILE_WRITE);
    if (file)
    {
        file.seek(0); // file_write includes append
        file.write(&log_index, sizeof(int));
        file.close();
    }
    else
    {
        PRINTLN("Error Open: SEQ_FILE file");
    }
}
/* ================ */
/* Days Operate     */
/* ================ */

bool fs_set_operation_time_file(uint16_t time)
{
    PRINTLN("Func: fs_set_operation_time_file()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }
    bool ret_value = false;

    File file = tnsyfs.open(OPERATE_FILE, FILE_WRITE);
    if (file)
    {
        file.seek(0); // file_write includes append
        file.write(&index, sizeof(uint16_t));
        file.close();
        ret_value = true;
    }
    else
    {
        PRINTLN("Error: open OPERATE_FILE file failed");
    }

    return ret_value;
}

uint16_t fs_get_operation_time_file()
{
    PRINTLN("Func: fs_get_operation_time_file()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    uint16_t time = 0;
    if (tnsyfs.exists(OPERATE_FILE))
    {
        File file = tnsyfs.open(OPERATE_FILE, FILE_READ);
        if (file)
        {
            file.read(&time, sizeof(uint16_t));
            file.close();
        }
        else
        {
            PRINTLN("Error: open OPERATE_FILE file failed");
        }
    }
    return time;
}

uint8_t fs_increase_operation_half_days_file()
{
    PRINTLN("Func: fs_increase_operation_half_days_file()");

    uint8_t error = 1;
    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }

    uint16_t time = 0;
    File file = tnsyfs.open(OPERATE_FILE, FILE_READ);
    if (file)
    {
        file.read(&time, sizeof(uint16_t));
        time = time + 1;
        file.write(&time, sizeof(uint16_t));
        file.close();
        error = 0;
    }
    else
    {
        PRINTLN("Error: open OPERATE_FILE file failed");
    }

    return error;
}

/* ================ */
/* Reset Index    */
/* ================ */

int fs_get_reset_index()
{
    PRINTLN("Func: fs_get_reset_index()");

    int index = 0;

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }
    else
    {
        if (tnsyfs.exists(RESET_FILE))
        {
            File file = tnsyfs.open(RESET_FILE, FILE_READ);
            if (file)
            {
                file.read(&index, sizeof(int));
                file.close();
                PRINT("Reset index:\t");
                PRINTLN(index);
            }
            else
            {
                PRINTLN("Error: open RESET_FILE file failed");
            }
        }
        else
        {
            PRINTLN("Info: RESET_FILE doesn't exist");
        }
    }
    return index;
}

void fs_set_reset_index(int index)
{
    PRINTLN("Func: fs_set_reset_index()");

    // Open root directory
    if (!tnsyfs.open("/"))
    {
        PRINTLN("tnsyfs.open() failed");
    }
    else
    {
        File file = tnsyfs.open(RESET_FILE, FILE_WRITE);
        if (file)
        {
            file.seek(0); // file_write includes append
            file.write(&index, sizeof(int));
            file.close();
        }
        else
        {
            PRINTLN("Error: open RESET_FILE file failed");
        }
    }
}

void fs_inc_reset_index()
{
    PRINTLN("Func: fs_inc_reset_index()");

    // read index
    int index = fs_get_reset_index();
    // increament
    index++;
    // save
    fs_set_reset_index(index);

    PRINT("Reset index:\t");
    PRINTLN(index);
}
#endif