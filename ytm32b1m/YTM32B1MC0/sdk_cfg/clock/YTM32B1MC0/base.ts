export function getVarColor(p: string) {
    const dark = document.documentElement.className.indexOf("dark") > -1;
    if (p == 'black') {
        if (dark) {
            return '#ffffff'
        } else {
            return '#000000'
        }
    } else if (p == 'white') {
        if (dark) {
            return '#000000'
        } else {
            return '#ffffff'
        }
    } else
        return getComputedStyle(document.documentElement).getPropertyValue(p).trim()
}

