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

        static UUID FromString(const CHAR* str){
            UINT8 bytes[16];
            INT32 byteidx = 0;
            INT32 stridx = 0;

            while(str[stridx] != '\0' && byteidx < 16){
                if(str[stridx] == '-'){
                    stridx++;
                    continue;
                }
                
                for(int nibble = 0; nibble < 2; nibble++){
                    CHAR c = str[stridx++];
                    UINT8 value = 0;

                    if(c >= '0' && c <= '9'){
                        value = c - '0';
                    } else if(c >= 'a' && c <= 'f'){
                        value = 10 + (c - 'a');
                    } else if(c >= 'A' && c <= 'F'){
                        value = 10 + (c - 'A');
                    } else {
                        return UUID{};
                    }
                    if(nibble == 0){
                        bytes[byteidx] = value << 4;
                    } else {
                       bytes[byteidx] = bytes[byteidx] | value;
                }

            } byteidx++;
        }
        return byteidx == 16 ? UUID(bytes) : UUID{};
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
