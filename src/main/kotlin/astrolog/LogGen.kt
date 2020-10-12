package astrolog

import java.io.File

data class LogEntryWithLine(val logEntry: LogEntry, val line: Int)

class ParseException(message: String) : Exception(message)


fun readLogEntriesFile(fileName: String): Map<String, LogEntry> {
    val fileLines = mutableListOf<String>()
    File(fileName).useLines { filelines -> fileLines.addAll(filelines) }

    val logEntries = mutableListOf<LogEntryWithLine>()
    fileLines.forEachIndexed { index, line ->
        if (!line.startsWith("#")) {
            val lineNumber = index + 1
            try {
                val observingObject = LogEntry.parse(line)
                logEntries.add(LogEntryWithLine(observingObject, lineNumber))
            } catch (e: Exception) {
                throw ParseException(e.message + " on line " + lineNumber)
            }
        }
    }

    return logEntries.map { x -> x.logEntry }.associateBy { e -> e.id }
}

fun readLogIdsFile(fileName: String, knownIds: Set<String>): List<String> {
    val fileLines = mutableListOf<String>()
    File(fileName).useLines { filelines -> fileLines.addAll(filelines) }

    val logIds = mutableListOf<String>()
    fileLines.forEachIndexed { index, line ->
        if (!line.startsWith("#")) {
            val lineNumber = index + 1
            val logId = line.trim()
            if (!knownIds.contains(logId)) {
                throw ParseException("unknown log id $logId on line $lineNumber")
            }
            logIds.add(logId)
        }
    }
    return logIds
}

fun readTemplateFile(fileName: String): String {
    return File(fileName).readText()
}

fun generateHtmlOutput(
    template: String,
    logIds: List<String>,
    logEntries: Map<String, LogEntry>,
    imageDir: String,
    imageExtension: String,
    name: String
): String {
    val replacement = logIds.map { id ->
        logEntries[id]!!.toHtmlTable(imageDir, imageExtension)
    }.joinToString("")

    return template.replace("%OBSERVATIONS%", replacement)
        .replace("%NAME%", name)
}

fun main(args: Array<String>) {
    try {
        val options = parseArgs(args)
        val logEntries = readLogEntriesFile(options.logFile)
        val knownIds = logEntries.keys
        val logIds = readLogIdsFile(options.logIdsFile, knownIds)
        val template = readTemplateFile(options.templateFile)
        val outputFile = options.outputFile
        val name = options.listName

        val generatedHtml = generateHtmlOutput(
            template, logIds, logEntries, options.imageDir, options.imageExtension, name)

        File(outputFile).writeText(generatedHtml)
        println("wrote " + logIds.size + " entries to " + outputFile)
    } catch (e: Exception) {
        println(e)
    }
}
