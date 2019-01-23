;
(function () {

    function count(element) {
        var i = 0;
        for (let index = 0; index < element.length; index++) {
            if (element.charAt(index) == '#')
                i++;
            else {
                break;
            }
        }
        return i - 2;
    }

    function gfm(element) {
        var array = [];
        for (let index = 0; index < element.length; index++) {
            const ch = element.charAt(index);
            if (ch == ' ') {
                array.push('-');
            } else {
                array.push(ch.toLowerCase());
            }
        }
        return array.join("");
    }

    function strip(element) {
        var i = element.indexOf("# ");
        if (i != -1) {
            element = element.substring(i + 2);
        }
        return element;
    }

    function title(value) {
        var lines = value.split('\n');
        var array = [];
        for (let index = 0; index < lines.length; index++) {
            const element = lines[index];
            var re = RegExp("^#{2,} +");
            var url = strip(element);
            if (re.test(element)) {
                array.push(`${"\t".repeat(count(element))} * [${url}](#${gfm(url) })`);
            }
        }
        return array.join("\n");
    }

    window["BuildTitle"] = title;
})();