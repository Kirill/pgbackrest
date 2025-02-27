/***********************************************************************************************************************************
Yaml Handler
***********************************************************************************************************************************/
#include "build.auto.h"

#include <yaml.h>

#include "common/debug.h"
#include "common/log.h"

#include "build/common/yaml.h"

/***********************************************************************************************************************************
Object type
***********************************************************************************************************************************/
struct Yaml
{
    yaml_parser_t parser;                                           // Parse context
};

/***********************************************************************************************************************************
Free parser context
***********************************************************************************************************************************/
static void
yamlFreeResource(THIS_VOID)
{
    THIS(Yaml);

    FUNCTION_LOG_BEGIN(logLevelTrace);
        FUNCTION_LOG_PARAM(YAML, this);
    FUNCTION_LOG_END();

    ASSERT(this != NULL);

    yaml_parser_delete(&this->parser);

    FUNCTION_LOG_RETURN_VOID();
}

/**********************************************************************************************************************************/
Yaml *
yamlNew(const Buffer *const buffer)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(BUFFER, buffer);
    FUNCTION_TEST_END();

    Yaml *this = NULL;

    OBJ_NEW_BEGIN(Yaml, .childQty = MEM_CONTEXT_QTY_MAX, .callbackQty = 1)
    {
        // Create object
        this = OBJ_NEW_ALLOC();
        *this = (Yaml){{0}};                                        // Extra braces are required for older gcc versions

        // Initialize parser context
        CHECK(ServiceError, yaml_parser_initialize(&this->parser), "unable to initialize yaml parser");
        memContextCallbackSet(objMemContext(this), yamlFreeResource, this);

        // Set yaml string
        yaml_parser_set_input_string(&this->parser, bufPtrConst(buffer), bufUsed(buffer));

        // Start document
        CHECK(FormatError, yamlEventNext(this).type == yamlEventTypeStreamBegin, "expected yaml stream begin");
        CHECK(FormatError, yamlEventNext(this).type == yamlEventTypeDocBegin, "expected yaml document begin");
    }
    OBJ_NEW_END();

    FUNCTION_TEST_RETURN_TYPE_P(Yaml, this);
}

/**********************************************************************************************************************************/
// Helper to map event type
static YamlEventType
yamlEventType(yaml_event_type_t type)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(ENUM, type);
    FUNCTION_TEST_END();

    switch (type)
    {
        case YAML_STREAM_START_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeStreamBegin);

        case YAML_STREAM_END_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeStreamEnd);

        case YAML_DOCUMENT_START_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeDocBegin);

        case YAML_DOCUMENT_END_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeDocEnd);

        case YAML_ALIAS_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeAlias);

        case YAML_SCALAR_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeScalar);

        case YAML_SEQUENCE_START_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeSeqBegin);

        case YAML_SEQUENCE_END_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeSeqEnd);

        case YAML_MAPPING_START_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeMapBegin);

        case YAML_MAPPING_END_EVENT:
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeMapEnd);

        default:
            CHECK(FormatError, type == YAML_NO_EVENT, "expected yaml no event");
            FUNCTION_TEST_RETURN_TYPE(YamlEventType, yamlEventTypeNone);
    }
}

YamlEvent
yamlEventNext(Yaml *this)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(YAML, this);
    FUNCTION_TEST_END();

    ASSERT(this != NULL);

    yaml_event_t event;

    if (!yaml_parser_parse(&this->parser, &event))
    {
        // These should always be set
        CHECK(ServiceError, this->parser.problem_mark.line && this->parser.problem_mark.column, "invalid yaml error info");

        THROW_FMT(
            FormatError, "yaml parse error: %s at line: %lu column: %lu", this->parser.problem,
            (unsigned long)this->parser.problem_mark.line + 1, (unsigned long)this->parser.problem_mark.column + 1);
    }

    YamlEvent result =
    {
        .type = yamlEventType(event.type),
        .line = event.start_mark.line + 1,
        .column = event.start_mark.column + 1,
    };

    if (result.type == yamlEventTypeScalar)
        result.value = strNewZ((const char *)event.data.scalar.value);

    yaml_event_delete(&event);

    FUNCTION_TEST_RETURN_TYPE(YamlEvent, result);
}

/**********************************************************************************************************************************/
YamlEvent
yamlEventNextCheck(Yaml *this, YamlEventType type)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(YAML, this);
        FUNCTION_TEST_PARAM(STRING_ID, type);
    FUNCTION_TEST_END();

    YamlEvent result = yamlEventNext(this);
    yamlEventCheck(result, type);

    FUNCTION_TEST_RETURN_TYPE(YamlEvent, result);
}

/**********************************************************************************************************************************/
void
yamlEventCheck(YamlEvent event, YamlEventType type)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(YAML_EVENT, event);
        FUNCTION_TEST_PARAM(STRING_ID, type);
    FUNCTION_TEST_END();

    if (event.type != type)
    {
        THROW_FMT(
            FormatError, "expected event type '%s' but got '%s' at line %zu, column %zu", strZ(strIdToStr(type)),
            strZ(strIdToStr(event.type)), event.line, event.column);
    }

    FUNCTION_TEST_RETURN_VOID();
}

/**********************************************************************************************************************************/
bool
yamlBoolParse(YamlEvent event)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(YAML_EVENT, event);
    FUNCTION_TEST_END();

    if (strEq(event.value, FALSE_STR))
        FUNCTION_TEST_RETURN(BOOL, false);
    else if (strEq(event.value, TRUE_STR))
        FUNCTION_TEST_RETURN(BOOL, true);

    THROW_FMT(FormatError, "invalid boolean '%s'", strZ(event.value));
}
