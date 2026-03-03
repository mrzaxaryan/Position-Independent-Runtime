#include "primitives.h"
#include "random.h"


class UUID {
    private:
        UINT8 data[16];
    public:
        UUID() { Memory::Zero(data, 16); }

        UUID(auto bytes){
           Memory::Copy(data, bytes, 16);
        }
        static UUID RandomUUID(){
            UUID uuid;
            Random rng; // Random class for random number generation

            for (INT32 i = 0; i < 16; i++){
                uuid.data[i] = static_cast<UINT8>(rng.Get() & 0xFF);
            }

            return uuid;
        }

        static UUID FromString(const char* str){
            UINT8 bytes[16] = {0};
            INT32 byteIndex = 0;
            INT32 count = 0;

            for(INT32 i = 0; str[i]!='\0'; i++){
                if(str[i] == '-') continue;

                UINT8 value = 0;
                CHAR c = str[i];
                if(c >= '0' && c <= '9') value = static_cast<UINT8>(c - '0');
                else if(c >= 'a' && c <= 'f') value = static_cast<UINT8>(c - 'a' + 10);
                else if(c >= 'A' && c <= 'F') value = static_cast<UINT8>(c - 'A' + 10);
                else continue;

                if(count == 0){
                    bytes[byteIndex] = static_cast<UINT8>(value << 4); // high nibble
                    count = 1;   
                } else {
                    bytes[byteIndex] |= value; // low nibble
                    byteIndex++;
                    count = 0;
                }
            }

            return UUID(bytes);
        }
   
        // toString method to convert the UUID to a string representation
        VOID ToString(const UUID* pUuid, char* buffer){
            const UINT8* bytes = pUuid->data;
            int pos = 0;

            for(int i =0; i< 16; i++){
                if(i == 4 || i == 6 || i == 8 || i == 10){
                    buffer[pos++] = '-';
                }
                const CHAR* hex = "0123456789abcdef"_embed;

                buffer[pos++] = hex[bytes[i] >> 4];
                buffer[pos++] = hex[bytes[i] & 0x0F];
            }
            buffer[pos] = '\0';
        }

        UINT64 GetMostSignificantBits(){
            UINT64 msb = 0;
            for(INT32 i = 0; i < 8; i++){
                msb = (msb << 8) | data[i];
            }
            return msb;
        }

        UINT64 GetLeastSignificantBits(){
            UINT64 lsb = 0;
            for(INT32 i = 8; i < 16; i++){
                lsb = (lsb << 8) | data[i];
            }
            return lsb;
        }
   
    };
