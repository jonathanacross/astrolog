package astrolog

import astrolog.ParseException

// TODO: For now, all items are basically strings.  In the future,
// make more concrete types and join with the astrodb data to
// do better checking.
data class LogEntry(
    val id: String,
    val date: String,
    val location: String,
    val scope: String,
    val seeing: String,
    val transparency: String,
    val objects: String,
    val time: String,
    val eyepiece: String,
    val magnification: String,
    val notes: String,
    val sketch: String
) {
    fun toHtmlTable(imageDir: String, imageExtension: String): String {
        fun escapeHtml(s: String): String {
            return s.replace("&", "&amp;")
                .replace("<", "&lt;")
                .replace(">", "&gt;")
                .replace("'", "&#39;")
                .replace("\"", "&quot;")
        }

        var obsInfo = ""
        if (date.isNotBlank()) { obsInfo += "<li>Date: $date</li> " }
        if (time.isNotBlank()) { obsInfo += "<li>Time: $time</li> " }
        if (location.isNotBlank()) { obsInfo += "<li>Location: $location</li> " }
        if (scope.isNotBlank()) { obsInfo += "<li>Scope: $scope</li> " }
        if (seeing.isNotBlank()) { obsInfo += "<li>Seeing: $seeing</li> " }
        if (transparency.isNotBlank()) { obsInfo += "<li>Transparency: $transparency</li> " }
        if (eyepiece.isNotBlank()) { obsInfo += "<li>Eyepiece: $eyepiece</li> " }
        if (magnification.isNotBlank()) { obsInfo += "<li>Magnification: $magnification</li> " }

        val htmlNotes = escapeHtml(notes)

        val objTemplate =
            """
            <div class=observation>
                <div class=notes>
                    <div class=objname>$objects</div>
                    <ul>
                        $obsInfo
                    </ul>
                    $htmlNotes
                </div>
                <div class=sketch>
                    <img src="$imageDir/$sketch$imageExtension">
                </div>
            </div>
            """.trimIndent()

        return objTemplate
    }

    fun toHtml(): String {
        var obsInfo = ""
        if (date.isNotBlank() || time.isNotBlank()) {
            obsInfo += """<tr><td>Date: $date</td> <td>Time: $time</td></tr>"""
        }
        if (location.isNotBlank() || scope.isNotBlank()) {
            obsInfo += """<tr><td>Location: $location</td><td>Scope: $scope</td></tr>"""
        }
        if (seeing.isNotBlank() || transparency.isNotBlank()) {
            obsInfo += """<tr><td>Seeing: $seeing</td><td>Transparency: $transparency</td></tr>"""
        }
        if (eyepiece.isNotBlank() || magnification.isNotBlank()) {
            obsInfo += """<tr><td>Eyepiece: $eyepiece</td><td>Magnification: $magnification</td></tr>""";
        }
        if (obsInfo.isNotBlank()) {
            obsInfo =
                """
                    <table>
                    $obsInfo
                    </table>
                """.trimIndent()
        }

        val notesString =
            if (notes.isNotBlank()) {
                """
                    <div class=notes>
                    $notes
                    </div>
                """.trimIndent()
            } else {
                ""
            }

        val objTemplate =
            """
            <div class=observation>
                <div class=objname>
                $objects
                </div>
                $obsInfo
                <img src="sketches/$sketch">
                $notesString
            </div>
            """.trimIndent()

        return objTemplate
    }

    companion object {
        fun parse(line: String): LogEntry {
            val fields = line.split("\t")
            val id = fields[0]
            val date = fields[1]
            val location = fields[2]
            val scope = fields[3]
            val seeing = fields[4]
            val transparency = fields[5]
            val objects = fields[6]
            val time = fields[7]
            val eyepiece = fields[8]
            val magnification = fields[9]
            val notes = fields[10]
            val sketch = fields[11]
            return LogEntry(
                id, date, location, scope, seeing,
                transparency, objects, time, eyepiece,
                magnification, notes, sketch
            )
        }
    }
}
