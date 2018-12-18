Date.prototype.yyyymmdd = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();
    
    return [this.getFullYear(),
            (mm>9 ? '' : '0') + mm,
            (dd>9 ? '' : '0') + dd
            ].join('');
};
console.log((new Date().yyyymmdd())+(time_format(new Date())))
function time_format(d) {
    let hours = format_two_digits(d.getHours());
    let minutes = format_two_digits(d.getMinutes());
    let seconds = format_two_digits(d.getSeconds());
    return hours + minutes + seconds;
}

function format_two_digits(n) {
    return n < 10 ? '0' + n : n;
}