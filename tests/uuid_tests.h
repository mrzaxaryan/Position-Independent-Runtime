#include "runtime/runtime.h"
#include "tests.h"
#include "core/types/uuid.h"

class UUIDTests
{
private:
    static BOOL TestUUIDGenerationAndParsing()
    {
        LOG_INFO("Test: UUID Generation and Parsing");

        // Generate a random UUID
        UUID uuid = UUID::RandomUUID();

        // Convert to string
        CHAR uuidStr[37]; // 36 chars + null terminator
        uuid.ToString(&uuid, uuidStr);

        LOG_INFO("Generated UUID: %s", uuidStr);

        // Parse back from string
        UUID parsedUuid = UUID::FromString(uuidStr);

        // Convert parsed UUID back to string for comparison
        CHAR parsedUuidStr[37];
        parsedUuid.ToString(&parsedUuid, parsedUuidStr);

        LOG_INFO("Parsed UUID: %s", parsedUuidStr);

        BOOL matches = (Memory::Compare(uuidStr, parsedUuidStr, 36) == 0);

        if (!matches)
        {
            LOG_ERROR("UUID mismatch!");
            return false;
        }

        return true;
    }

    public:
        static BOOL RunAll()
        {
            BOOL allPassed = true;

            LOG_INFO("Running UUID Tests...");

            RunTest(allPassed, EMBED_FUNC(TestUUIDGenerationAndParsing), "UUID generation and parsing"_embed);

            if (allPassed) LOG_INFO("All UUID tests passed!");
            else LOG_ERROR("Some UUID tests failed!");

            return allPassed;
        }   
};