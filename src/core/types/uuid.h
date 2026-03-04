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

        static UUID FromString(const CHAR* str, UUID* pUuid){
            UINT8 bytes[16];
            INT32 byteidx = 0;

            for (INT32 i = 0; i < 36; i++){
                if (str[i] == '-'){
                    continue;
                }
    
                const CHAR* hex = "0123456789abcdef"_embed;
                UINT8 high = 0, low = 0;

                for (INT32 j = 0; j < 16; j++){
                    if (str[i] == hex[j]){
                        high = j;
                    }
                    if (str[i+1] == hex[j]){
                        low = j;
                    }
                }
                bytes[byteidx++] = (high << 4) | low;
                i++; 
            }

            if (pUuid) {
                Memory::Copy(pUuid->data, bytes, 16);
            }
            
            return *pUuid;
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
