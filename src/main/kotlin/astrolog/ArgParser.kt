package astrolog

import astrolog.ParseException

data class Options(
    val logFile: String,
    val logIdsFile: String,
    val templateFile: String,
    val listName: String,
    val imageDir: String,
    val imageExtension: String,
    val outputFile: String
)

fun usage(): String {
    return """
        Usage: java -jar LogGen.jar [options]
            --log={tsv log file}
            --logIds={log ids file}
            --template={html template file}
            --name={name of list}
            --imageDir={directory of images, default='sketches'}
            --imageExtension={default='.jpg'}
            --output={html output file}
    """.trimIndent()
}

// Small hack to expand ~ to user directory to allow user to specify files as ~/....
private fun expandPath(path: String): String {
    return path.replaceFirst(Regex("^~"), System.getProperty("user.home"))
}

fun parseArgs(args: Array<String>): Options {
    var logFileName = ""
    var logIdsFileName = ""
    var templateFileName = ""
    var listName = "Observing List"
    var imageDir = "sketches"
    var imageExtension = ".jpg"
    var outputFileName = ""

    for (arg in args) {
        val keyValue = arg.split("=", limit = 2)
        if (keyValue.size != 2) {
            throw ParseException("couldn't parse '$keyValue'\n." + usage())
        }
        val key = keyValue[0]
        val value = keyValue[1]
        when (key) {
            "--log" -> logFileName = expandPath(value)
            "--logIds" -> logIdsFileName = expandPath(value)
            "--template" -> templateFileName = expandPath(value)
            "--name" -> listName = value
            "--imageDir" -> imageDir = value
            "--imageExtension" -> imageExtension = value
            "--output" -> outputFileName = expandPath(value)
            else -> throw ParseException("unknown argument $key. " + usage())
        }
    }

    // simple validation
    if (logFileName.isBlank()) {
        throw ParseException("--log file not specified")
    }
    if (logIdsFileName.isBlank()) {
        throw ParseException("--logIds file not specified")
    }
    if (templateFileName.isBlank()) {
        throw ParseException("--template file not specified")
    }
    if (outputFileName.isBlank()) {
        throw ParseException("--output file not specified")
    }
    return Options(
        logFile = logFileName,
        logIdsFile = logIdsFileName,
        templateFile = templateFileName,
        listName = listName,
        imageDir = imageDir,
        imageExtension = imageExtension,
        outputFile = outputFileName
    )
}
