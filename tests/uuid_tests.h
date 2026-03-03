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

    static BOOL TestToStringFunction()
    {
        LOG_INFO("Test: UUID ToString Function");

        // Create a known UUID
        UUID uuid = UUID::FromString("12345678-9abc-def0-1122-334455667788"_embed);

        // Convert to string
        CHAR uuidStr[37];
        uuid.ToString(&uuid, uuidStr);

        LOG_INFO("UUID String: %s", uuidStr);

        // Expected string representation
        const char* expectedStr = "12345678-9abc-def0-1122-334455667788"_embed;

        BOOL matches = (Memory::Compare(uuidStr, expectedStr, 36) == 0);

        if (!matches)
        {
            LOG_ERROR("ToString output mismatch!");
            return false;
        }

        return true;
    }

    static BOOL TestFromStringFunction()
    {
        LOG_INFO("Test: UUID FromString Function");

        // Known UUID string
        const char* uuidStr = "12345678-9abc-def0-1122-334455667788"_embed;

        // Parse UUID
        UUID uuid = UUID::FromString(uuidStr);

        // Convert back to string
        CHAR parsedUuidStr[37];
        uuid.ToString(&uuid, parsedUuidStr);

        LOG_INFO("Parsed UUID String: %s", parsedUuidStr);

        BOOL matches = (Memory::Compare(uuidStr, parsedUuidStr, 36) == 0);

        if (!matches)
        {
            LOG_ERROR("FromString output mismatch!");
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
            RunTest(allPassed, EMBED_FUNC(TestToStringFunction), "UUID ToString function"_embed);
            RunTest(allPassed, EMBED_FUNC(TestFromStringFunction), "UUID FromString function"_embed);
            if (allPassed) LOG_INFO("All UUID tests passed!");
            else LOG_ERROR("Some UUID tests failed!");

            return allPassed;
        }   
};