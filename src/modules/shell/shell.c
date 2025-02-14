#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/shell/shell.h"
#include "util/stringUtils.h"

#define FF_SHELL_NUM_FORMAT_ARGS 6

void ffPrintShell(FFShellOptions* options)
{
    const FFShellResult* result = ffDetectShell();

    if(result->processName.length == 0)
    {
        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, "Couldn't detect shell");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result->prettyName, stdout);

        if(result->version.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&result->version, stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->exe},
            {FF_FORMAT_ARG_TYPE_STRING, result->exeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->version},
            {FF_FORMAT_ARG_TYPE_UINT, &result->pid},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->prettyName},
        });
    }
}

bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SHELL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseShellJsonObject(FFShellOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateShellJsonConfig(FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyShellOptions))) FFShellOptions defaultOptions;
    ffInitShellOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateShellJsonResult(FF_MAYBE_UNUSED FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFShellResult* result = ffDetectShell();

    if(result->processName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect shell");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result->exe);
    yyjson_mut_obj_add_strcpy(doc, obj, "exeName", result->exeName);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result->pid);
    yyjson_mut_obj_add_uint(doc, obj, "ppid", result->ppid);
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->processName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->prettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->version);
}

void ffPrintShellHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_SHELL_MODULE_NAME, "{3} {4}", FF_SHELL_NUM_FORMAT_ARGS, (const char* []) {
        "Shell process name",
        "Shell path with exe name",
        "Shell exe name",
        "Shell version",
        "Shell pid",
        "Shell pretty name"
    });
}

void ffInitShellOptions(FFShellOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_SHELL_MODULE_NAME,
        "Print current shell name and version",
        ffParseShellCommandOptions,
        ffParseShellJsonObject,
        ffPrintShell,
        ffGenerateShellJsonResult,
        ffPrintShellHelpFormat,
        ffGenerateShellJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyShellOptions(FFShellOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
