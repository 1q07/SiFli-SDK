# DFU_PAN Firmware Packaging Tool User Guide

## 1. Overview of the Tool
Firmware Upgrade (DFU_PAN, Device Firmware Update PAN):  
This firmware upgrade method enables OTA firmware download and installation via Bluetooth PAN connection. This document introduces how to use this script tool to package firmware.  
Script Tool: `"$SIFLI_SDK/tool/dfu_pan_tool"`  
Used Middleware: `"$SIFLI_SDK/middleware/dfu_pan"`

The OTA firmware packaging tool is a Python script designed to bundle multiple bin files into a single OTA firmware package. The tool supports flexible INI configuration file formats, uniformly using little-endian storage, with the magic number corresponding to "_OTA" (0x5F4F5441).

## 2. Usage Method

### 2.1 Command Line Parameters

```bash
python dfu_pan_paket.py --ini <INI Configuration File Path> [--output <Output OTA Package Path>]
```

- `--ini`: Required parameter, specifies the path to the INI configuration file.
- `--output`: Optional parameter, specifies the output OTA package path; default is `ota_mix.bin`.

### 2.2 Example

```bash
python dfu_pan_paket.py --ini ./dfu_pan.ini --output ./ota_package.bin
```

## 3. INI Configuration File Description

The INI configuration file consists of two parts: the COMMON section and individual bin file sections.

### 3.1 COMMON Section

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # Magic number, fixed at 0x5F4F5441 (_OTA)
IMG_VER=0x00000134     # Version number, example: 1.3.4
FILE_PATH=./bin_files  # Directory path where bin files are located
```

### 3.2 Bin File Sections

Each bin file requires an independent section, which can be named arbitrarily. Below are descriptions of each field:

```ini
[APP]                  # Custom section name
NAME=ER_IROM1.bin      # Name of the bin file
SEL=1                  # Whether selected (1=selected, 0=not selected)
GZIP=0                 # Whether compressed (1=compressed, 0=not compressed)
ID=0                   # File ID
ADDR=0x12218000        # Flash address
REGION_SIZE=0x00240000 # Erase region size
```

Multiple bin file sections can be configured, for example:

```ini
[COMMON]
IMG_FLAG=0x5F4F5441
IMG_VER=0x00000134
FILE_PATH=./bin_files

[APP]
NAME=ER_IROM1.bin
SEL=1
GZIP=0
ID=0
ADDR=0x12218000
REGION_SIZE=0x00240000

[IMG]
NAME=ER_IROM3.bin
SEL=1
GZIP=0
ID=1
ADDR=0x12460000
REGION_SIZE=0x00680000

[FONT]
NAME=ER_IROM2.bin
SEL=1
GZIP=0
ID=2
ADDR=0x12AE0000
REGION_SIZE=0x00400000
```

## 4. Firmware Package Structure

Based on the provided INI configuration file example, the generated OTA firmware package has the following structure:

```
+----------------------+ <- 0x00000000
| Total Header (16 bytes) |
| - Header CRC32 (4 bytes) | 0xF16C8B4A
| - Magic Number (4 bytes) | 0x5F4F5441 (_OTA)
| - Version Number (4 bytes)| 0x00000134 (1.3.4)
| - Number of Bin Files (4 bytes)| 0x00000003 (3 files)
+----------------------+ <- 0x00000010
| Bin File 1 Header (76 bytes) |
| - Filename (48 bytes) | "ER_IROM1.bin\0\0\0..."
| - File ID (2 bytes) | 0x0000
| - GZIP Flag (2 bytes) | 0x0000
| - Data Length (4 bytes) | [Actual file size]
| - Original Length (4 bytes) | [Actual file size]
| - Flash Address (4 bytes) | 0x12218000
| - Data CRC32 (4 bytes) | [Calculated based on actual content]
| - Original Data CRC32 (4 bytes)| [Calculated based on actual content]
| - Region Size (4 bytes) | 0x00240000
+----------------------+ <- [Offset]
| Bin File 2 Header (76 bytes) |
| - Filename (48 bytes) | "ER_IROM3.bin\0\0\0..."
| - File ID (2 bytes) | 0x0001
| - GZIP Flag (2 bytes) | 0x0000
| - Data Length (4 bytes) | [Actual file size]
| - Original Length (4 bytes) | [Actual file size]
| - Flash Address (4 bytes) | 0x12460000
| - Data CRC32 (4 bytes) | [Calculated based on actual content]
| - Original Data CRC32 (4 bytes)| [Calculated based on actual content]
| - Region Size (4 bytes) | 0x00680000
+----------------------+ <- [Offset]
| Bin File 3 Header (76 bytes) |
| - Filename (48 bytes) | "ER_IROM2.bin\0\0\0..."
| - File ID (2 bytes) | 0x0002
| - GZIP Flag (2 bytes) | 0x0000
| - Data Length (4 bytes) | [Actual file size]
| - Original Length (4 bytes) | [Actual file size]
| - Flash Address (4 bytes) | 0x12AE0000
| - Data CRC32 (4 bytes) | [Calculated based on actual content]
| - Original Data CRC32 (4 bytes)| [Calculated based on actual content]
| - Region Size (4 bytes) | 0x00400000
+----------------------+ <- [Offset]
| Bin File 1 Data |
| ([Actual file size] bytes) |
+----------------------+ <- [Offset]
| Bin File 2 Data |
| ([Actual file size] bytes) |
+----------------------+ <- [Offset]
| Bin File 3 Data |
| ([Actual file size] bytes) |
+----------------------+ <- [File End]
```

### 4.1 Total Header Structure (16 Bytes)

| Offset | Field Name | Size | Description |
|--------|------------|------|-------------|
| 0x00 | Header CRC32 | 4 bytes | CRC32 checksum of all header information |
| 0x04 | Magic Number | 4 bytes | Fixed value 0x5F4F5441 ('_OTA') |
| 0x08 | Version Number | 4 bytes | Firmware version number |
| 0x0C | Number of Bin Files | 4 bytes | Number of included bin files |

### 4.2 Bin File Header Structure (76 Bytes)

Each bin file has a 76-byte header containing the following information:

| Offset | Field Name | Size | Description |
|--------|------------|------|-------------|
| 0x00 | Filename | 48 bytes | Bin filename, padded with '\0' if shorter |
| 0x30 | File ID | 2 bytes | File identifier |
| 0x32 | GZIP Flag | 2 bytes | Compression flag, 0 = uncompressed, 1 = compressed |
| 0x34 | Data Length | 4 bytes | Actual data length (compressed length) |
| 0x38 | Original Length | 4 bytes | Original data length (after alignment) |
| 0x3C | Flash Address | 4 bytes | Target address in Flash |
| 0x40 | Data CRC32 | 4 bytes | CRC32 checksum of the data |
| 0x44 | Original Data CRC32 | 4 bytes | CRC32 checksum of the original data |
| 0x48 | Region Size | 4 bytes | Flash erase region size |

### 4.3 Header CRC32 Calculation Process

The header CRC32 calculation is based on the following data:
1. Magic number: 0x5F4F5441 (little-endian: 0x41, 0x54, 0x4F, 0x5F)
2. Version number: 0x00000134 (little-endian: 0x34, 0x01, 0x00, 0x00)
3. Number of bin files: 0x00000003 (little-endian: 0x03, 0x00, 0x00, 0x00)
4. All bin file header information

Assuming the three bin file headers are as follows (in little-endian format):

**Bin File 1 Header (ER_IROM1.bin):**
- Filename: "ER_IROM1.bin\0\0\0..." (48 bytes)
- File ID: 0x0000 (little-endian: 0x00, 0x00)
- GZIP Flag: 0x0000 (little-endian: 0x00, 0x00)
- Data Length: [actual value] (little-endian)
- Original Length: [actual value] (little-endian)
- Flash Address: 0x12218000 (little-endian: 0x00, 0x80, 0x21, 0x12)
- Data CRC32: [actual value] (little-endian)
- Original Data CRC32: [actual value] (little-endian)
- Region Size: 0x00240000 (little-endian: 0x00, 0x00, 0x24, 0x00)

**Bin File 2 Header (ER_IROM3.bin):**
- Filename: "ER_IROM3.bin\0\0\0..." (48 bytes)
- File ID: 0x0001 (little-endian: 0x01, 0x00)
- GZIP Flag: 0x0000 (little-endian: 0x00, 0x00)
- Data Length: [actual value] (little-endian)
- Original Length: [actual value] (little-endian)
- Flash Address: 0x12460000 (little-endian: 0x00, 0x00, 0x46, 0x12)
- Data CRC32: [actual value] (little-endian)
- Original Data CRC32: [actual value] (little-endian)
- Region Size: 0x00680000 (little-endian: 0x00, 0x00, 0x68, 0x00)

**Bin File 3 Header (ER_IROM2.bin):**
- Filename: "ER_IROM2.bin\0\0\0..." (48 bytes)
- File ID: 0x0002 (little-endian: 0x02, 0x00)
- GZIP Flag: 0x0000 (little-endian: 0x00, 0x00)
- Data Length: [actual value] (little-endian)
- Original Length: [actual value] (little-endian)
- Flash Address: 0x12AE0000 (little-endian: 0x00, 0x00, 0xAE, 0x12)
- Data CRC32: [actual value] (little-endian)
- Original Data CRC32: [actual value] (little-endian)
- Region Size: 0x00400000 (little-endian: 0x00, 0x00, 0x40, 0x00)

Concatenate all the above data sequentially into a byte stream, then calculate the CRC value using the CRC32 algorithm (polynomial 0xEDB88320).

Specific calculation steps:
1. Initialize CRC to 0xffffffff
2. Process each byte:
   - crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF]
3. Final result is crc ^ 0xFFFFFFFF

The calculated header CRC32 value is: 0xF16C8B4A

## 5. Notes

1. Ensure all bin files exist in the directory specified in the INI configuration file.
2. Flash addresses and region sizes must be set according to actual hardware configuration.
3. If compression is enabled (GZIP=1), the tool will automatically perform GZIP compression, but the C code parsing end does not support it—do not select compression!
4. The tool automatically calculates and verifies CRC32 checksums to ensure data integrity.
5. All data uses little-endian storage format.
6. Bin file data is aligned to 16 bytes before writing, with padding of 0xFF if necessary.
7. Server-side folder order should place higher versions first to ensure devices correctly obtain the latest version.
8. The uploaded firmware package filename must match the folder name it belongs to, so that the correct download link can be generated in the JSON response.

## 6. OTA Firmware Package Parsing Matching Requirements

### 6.1 CRC32 Algorithm Consistency

The CRC32 algorithm used in the firmware package parsing code must be exactly consistent with the Python packing script:

```c
// C code implementation of CRC32
#define CRC32_POLY 0xEDB88320
static uint32_t crc32_table[256];

void init_crc32_table(void)
{
    for (int i = 0; i < 256; i++)
    {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
}

uint32_t calculate_crc32(const uint8_t *data, size_t length, uint32_t crc)
{
    crc = crc ^ 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}
```

```python
# Python script implementation of CRC32
CRC32_POLY = 0xEDB88320
crc32_table = []

def init_crc32_table():
    global crc32_table
    if crc32_table:
        return
    
    for i in range(256):
        crc = i
        for j in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ CRC32_POLY
            else:
                crc >>= 1
        crc32_table.append(crc)

def calculate_crc32(data: bytes, crc: int = 0xffffffff) -> int:
    init_crc32_table()
    crc = crc ^ 0xFFFFFFFF
    for byte in data:
        crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF]
    return crc ^ 0xFFFFFFFF
```

These implementations must be completely identical; otherwise, the firmware package integrity cannot be verified correctly.

### 6.2 Firmware Package Header Structure Matching

The `struct firmware_info` structure in C code must exactly match the header structure generated by the Python script:

```c
// C code firmware info structure
struct firmware_info {
    char name[48];              // 48 bytes
    uint16_t id;                // 2 bytes
    uint16_t gzip;              // 2 bytes
    uint32_t compressed_len;    // 4 bytes - compressed length
    uint32_t orig_length;       // 4 bytes - original length
    uint32_t addr;              // 4 bytes
    uint32_t data_crc;          // 4 bytes
    uint32_t data_oricrc;       // 4 bytes
    uint32_t region_size;       // 4 bytes
};
```

```python
# Python script packing format
bin_header = struct.pack(
    '<48sHHIIIIII',
    name_bytes,
    bin_file.id,
    gzip_flag,
    bin_file.compressed_len,
    bin_file.orig_len,
    bin_file.addr,
    bin_file.data_crc,
    bin_file.orig_crc,
    bin_file.region_size
)
```

### 6.3 Magic Number Matching

```c
// C code magic number verification
const uint32_t EXPECTED_MAGIC = 0x5F4F5441; // _OTA
if (magic != EXPECTED_MAGIC)
{
    // Handle mismatched magic number
}
```

```python
# Python script magic number setting
self.magic = magic  # Default 0x5F4F5441, corresponding to _OTA
```

### 6.4 Byte Order Consistency

Both C code and Python script use little-endian byte order (`<` in Python indicates little-endian):

```c
// C code reads data in little-endian
rt_memcpy(&headers_crc, buffer, 4);
rt_memcpy(&magic, buffer + 4, 4);
rt_memcpy(&version, buffer + 8, 4);
rt_memcpy(&bin_num, buffer + 12, 4);
```

```python
# Python script packs data in little-endian
total_header = struct.pack(
    '<IIII',
    header_crc,
    self.magic,
    self.version,
    len(self.bin_files)
)
```

### 6.5 Header Size Matching

Each bin file header must be exactly 76 bytes:

```
48 (filename) + 2 (ID) + 2 (gzip flag) + 4 (data length) + 4 (original length) 
+ 4 (address) + 4 (data CRC) + 4 (original CRC) + 4 (region size) = 76 bytes
```

### 6.6 INI Configuration Parameter Matching

Parameters in the INI configuration file directly affect the content of the generated firmware package, and must be matched during parsing:

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # Must match EXPECTED_MAGIC in C code
IMG_VER=0x00000134     # Version number

[APP]
NAME=ER_IROM1.bin      # Filename, stored in firmware_info.name
SEL=1                  # Selection flag, affects whether included in firmware package
GZIP=0                 # Compression flag, affects gzip field
ID=0                   # File ID, stored in firmware_info.id
ADDR=0x12218000        # Flash address, stored in firmware_info.addr
REGION_SIZE=0x00240000 # Region size, stored in firmware_info.region_size
```

### 6.7 Overall Structure Matching

The overall structure of the firmware package must remain consistent between C code parsing and Python script generation:

1. Total header (16 bytes): Header CRC (4) + Magic (4) + Version (4) + File count (4)
2. Each bin file header (76 bytes): Arranged according to firmware_info structure
3. Each bin file data: Stored according to compressed_len field length

## 7. Complete OTA Upgrade Usage Documentation

### 7.1 Generate Firmware Package

#### 7.1.1 Preparation

First, prepare the following files:
1. Multiple bin files (e.g., ER_IROM1.bin, ER_IROM3.bin, ER_IROM2.bin) (number of bin files corresponds to MAX_FIRMWARE_FILES in header file)
2. INI configuration file

#### 7.1.2 Create INI Configuration File

Create an INI configuration file, for example [dfu_pan.ini](file:///Volumes/TB/solution2.0/solution2.0/solution/tools/dfu_pan/dfu_pan.ini):

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # Magic number, fixed at 0x5F4F5441 (_OTA)
IMG_VER=0x00000134     # Version number, example: 1.3.4
FILE_PATH=./bin_files  # Directory path where bin files are located

[APP]
NAME=ER_IROM1.bin      # Bin filename
SEL=1                  # Whether selected (1=selected, 0=not selected)
GZIP=0                 # Whether compressed (1=compressed, 0=not compressed)
ID=0                   # File ID
ADDR=0x12218000        # Flash address
REGION_SIZE=0x00240000 # Erase region size

[IMG]
NAME=ER_IROM3.bin
SEL=1
GZIP=0
ID=1
ADDR=0x12460000
REGION_SIZE=0x00680000

[FONT]
NAME=ER_IROM2.bin
SEL=1
GZIP=0
ID=2
ADDR=0x12AE0000
REGION_SIZE=0x00400000
```

#### 7.1.3 Run Packaging Tool

Use the Python script to generate the OTA firmware package (name the firmware package according to version numbers for easy server deployment):

```bash
python dfu_pan_paket.py --ini ./dfu_pan.ini --output ./vx.x.x.bin
```

The generated firmware package will include all specified bin files and their metadata.

### 7.2 Deploy on Server

#### 7.2.1 Upload Firmware Package

Upload the generated OTA firmware package to the OTA server. Depending on your server configuration, you can use one of the following methods:

#### 7.2.2 Configure Server

Ensure the server is properly configured so that devices can access the firmware package:

File upload URL: https://ota.sifli.com/browser/
Example: https://ota.sifli.com/browser/xiaozhi/SF32LB52_EEP_NOR_TFT_CO5300/S2_watch_sf32lb52-ulp/v1.3.6?role=zhenpengfu
After "browser/", the path represents each layer of folder names. Upload v1.3.6.bin (must have the same name as the current folder to generate the corresponding JSON package URL)

Return JSON package URL: https://ota.sifli.com/v2/xiaozhi/SF32LB52_EEP_NOR_TFT_CO5300/S2_watch_sf32lb52-ulp (change "browser" to "v2")
The returned "zippath": is a template, and the C code parses and downloads by concatenating the returned template URL.

### 7.3 Independent OTA Program Execution Parsing and Burning

#### 7.3.1 Device-Side OTA Process

The device-side OTA upgrade process includes the following steps:

##### 7.3.1.1 Query Available Versions

The device first queries available firmware versions from the OTA server:

```c
// Query available versions
dfu_pan_query_versions(); 
```

This function will:
1. Send a GET request to the server
2. Parse the JSON data returned by the server
3. Extract up to 5 available version information (folder order should place higher versions first)
4. Store version information in Flash

##### 7.3.1.2 Enter Bootloader
Path: sdk/example/boot_loader
After executing the version check function, version information will be stored in Flash, and the bootloader will check the update flag to determine whether to enter the OTA boot program

**Note:** 
- DFU_PAN_LOADER_START_ADDR address is defined as 0xFFFFFFFF by default in `dfu_pan_macro.h`, need to check if DFU_PAN_LOADER segment is defined in the corresponding development board's ptab.json.
- In boot_loader, it will only jump to the OTA boot program if DFU_PAN_LOADER_START_ADDR != DFU_PAN_FLASH_UNINIT_32 (0xFFFFFFFF)

```c
run_img(DFU_PAN_LOADER_START_ADDR);
```

##### 7.3.1.3 OTA Boot Program
Path: sdk/example/dfu_pan
The OTA boot program has a simple UI interface, mainly used for downloading firmware packages and parsing, guiding users through firmware upgrades
Clicking the version list button lists the versions returned by the server (query available versions)

Clicking the corresponding version number enters the download process, which executes:
```c
// Start OTA upgrade thread
dfu_pan_create_worker_thread(version_index);
```

##### 7.3.1.4 Download and Parse Firmware Package

After the user selects the version to upgrade, the system starts the OTA upgrade process:
In the OTA worker thread:

```c
// Download and parse firmware package
dfu_pan_parse_package_from_url(version_list[version_index].zippath);
```

This function will:
1. Download the firmware package from the URL
2. Parse the firmware package header information
3. Verify the magic number and header CRC32
4. Parse the header information of each bin file
5. Store firmware information in Flash
6. Download and burn bin file data to Flash sequentially
7. Verify CRC32 of each bin file

##### 7.3.1.5 Detailed Burning Process

The firmware package parsing and burning process is as follows:

1. **Parse Total Header**:
   - Read 16 bytes of total header
   - Verify magic number is 0x5F4F5441 ('_OTA')
   - Read version number and number of bin files

2. **Parse Each Bin File Header**:
   - Read each bin file's 76-byte header sequentially
   - Extract filename, ID, compression flag, length, address, CRC, etc.
   - Store this information in Flash

3. **Burn Bin File Data**:
   - Read each bin file's data sequentially
   - Erase target Flash region
   - Write data to Flash
   - Display burning progress

4. **Verify CRC**:
   - Calculate CRC32 of burned data
   - Compare with CRC32 stored in the header
   - If mismatched, report error and stop upgrade

#### 7.3.2 Error Handling

Possible errors during OTA process and handling methods:

1. **Network Errors**:
   - Retry download
   - If multiple retries fail, report network error

2. **CRC Check Failure**:
   - Stop upgrade process
   - Report CRC check error

3. **Flash Operation Failure**:
   - Report Flash write/erase error
   - Try rollback operation

4. **Firmware Package Format Error**:
   - Verify magic number
   - Check file count and size
   - Report format error

#### 7.3.3 Post-Upgrade Processing

After upgrade completion:
1. Clear upgrade flag
2. Display success/failure message
3. Restart device as needed

```c
// Processing after successful upgrade
if (result == 0) {
    dfu_pan_clear_update_flags();  // Clear update flag
    rt_thread_mdelay(50);
    dfu_pan_ui_update_message(UI_MSG_SHOW_SUCCESS_POPUP, NULL);  // Show success message
} else {
    dfu_pan_ui_update_message(UI_MSG_SHOW_FAILURE_POPUP, NULL);  // Show failure message
}
```