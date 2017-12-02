

// void MP3Task(void *p)
// {
//     memset(Buffer, 0, sizeof(Buffer));
//     ssp0_init(0);

//     const uint8_t reset = 0;
//     const uint8_t dreq  = 1;
//     const uint8_t xcs   = 29;
//     const uint8_t xdcs  = 30;

//     // Set direction
//     LPC_GPIO0->FIODIR |= (1 << reset);
//     LPC_GPIO0->FIODIR &= ~(1 << dreq);
//     LPC_GPIO0->FIODIR |= (1 << xcs);
//     LPC_GPIO0->FIODIR |= (1 << xdcs);

//     // Set initial state
//     LPC_GPIO0->FIOSET |= (1 << xcs);
//     LPC_GPIO0->FIOSET |= (1 << xdcs);
//     LPC_GPIO0->FIOCLR |= (1 << reset);
//     vTaskDelay(1 / portTICK_PERIOD_MS);
//     LPC_GPIO0->FIOSET |= (1 << reset);
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );

//     const uint16_t default_status = 0x4800; // line1, native spi, stream, reset, allow mpeg 1+2
//     const uint16_t default_clock  = 0x6000;
//     const uint16_t default_volume = 0x2020;

//     // MODE
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//     LPC_GPIO0->FIOCLR |= (1 << xcs);
//     ssp0_exchange_byte(0x02);
//     ssp0_exchange_byte(0x0);
//     ssp0_exchange_byte(default_status >> 8);
//     ssp0_exchange_byte(default_status & 0xFF);
//     LPC_GPIO0->FIOSET |= (1 << xcs);
//     // CLOCKF
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//     LPC_GPIO0->FIOCLR |= (1 << xcs);
//     ssp0_exchange_byte(0x02);
//     ssp0_exchange_byte(0x3);
//     ssp0_exchange_byte(default_clock >> 8);
//     ssp0_exchange_byte(default_clock & 0xFF);
//     LPC_GPIO0->FIOSET |= (1 << xcs);
//     // VOL
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//     LPC_GPIO0->FIOCLR |= (1 << xcs);
//     ssp0_exchange_byte(0x02);
//     ssp0_exchange_byte(0xB);
//     ssp0_exchange_byte(default_volume >> 8);
//     ssp0_exchange_byte(default_volume & 0xFF);
//     LPC_GPIO0->FIOSET |= (1 << xcs);
//     // DECODE_TIME
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//     LPC_GPIO0->FIOCLR |= (1 << xcs);
//     ssp0_exchange_byte(0x02);
//     ssp0_exchange_byte(0x4);
//     ssp0_exchange_byte(0);
//     ssp0_exchange_byte(0);
//     LPC_GPIO0->FIOSET |= (1 << xcs);
//     // DECODE_TIME
//     while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//     LPC_GPIO0->FIOCLR |= (1 << xcs);
//     ssp0_exchange_byte(0x02);
//     ssp0_exchange_byte(0x4);
//     ssp0_exchange_byte(0);
//     ssp0_exchange_byte(0);
//     LPC_GPIO0->FIOSET |= (1 << xcs);

//     // Wait for button trigger
//     while ( !(Button0::getInstance().IsPressed()) );

//     FRESULT result;
//     UINT current_segment_size = 0;

//     // Main loop
//     while (1)
//     {
//         printf("[MP3Task] Opening file...\n");
//         // Open mp3 file
//         const char name[] = "1:track1.mp3";
//         result = f_open(&mp3_file, name, FA_OPEN_EXISTING | FA_READ);
//         if (result != FR_OK)
//         {
//             printf("[MP3Task] mp3 file failed to open. Error: %d\n", result);
//             break;
//         }
//         else
//         {
//             printf("[MP3Task] %s successfully opened.\n", name);
//         }
        
//         // Read a segment at a time before sending to device
//         current_segment_size = MP3_SEGMENT_SIZE;

//         // While reading max segment size, keep going
//         while (current_segment_size >= MP3_SEGMENT_SIZE)
//         {
//             // Read a segment
//             result = f_read(&mp3_file, Buffer, MP3_SEGMENT_SIZE, &current_segment_size);
//             if (result != FR_OK)
//             {
//                 printf("[MP3Task] mp3 file failed to read. Error: %d\n", result);
//                 break;
//             }

//             uint32_t cycles    = current_segment_size / 32;
//             uint16_t remainder = current_segment_size % 32;

//             // Wait for DREQ
//             while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//             // XDCS LOW
//             LPC_GPIO0->FIOCLR |= (1 << xdcs);

//             // Send bytes for each cycle
//             for (uint32_t cycle=0; cycle<cycles; cycle++)
//             {
//                 for (int byte=0; byte<32; byte++)
//                 {
//                     ssp0_exchange_byte(Buffer[cycle * 32 + byte]);
//                 }
//                 // Wait for DREQ
//                 while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//             }

//             // Send remainder
//             if (remainder > 0)
//             {
//                 for (int byte=0; byte<remainder; byte++)
//                 {
//                     // Cycles instead of cycle because it is after the last cycle
//                     ssp0_exchange_byte(Buffer[cycles * 32 + byte]);
//                 }
//                 // Wait for DREQ
//                 while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
//             }

//             // XDCS HIGH
//             LPC_GPIO0->FIOSET |= (1 << xdcs);
//         }

//         // Close the damn file
//         result = f_close(&mp3_file);
//         if (result != FR_OK) printf("[MP3Task] mp3 file failed to close!\n");
//         else                 printf("[MP3Task] mp3 file closed.\n");

//         // Wait 10 seconds
//         vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
//     }

//     printf("[MP3Task] MP3 file failed so task is suspending...\n");
//     vTaskSuspend(NULL);
// }