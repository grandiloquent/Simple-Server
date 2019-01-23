function alphabet() {
    var array = [];
    for (let n = 0; n < 26; n++) {
        var s = String.fromCharCode(97 + n)
        array.push(`${s}=0`);
    }
    console.log(array.join(','));
}
generate();