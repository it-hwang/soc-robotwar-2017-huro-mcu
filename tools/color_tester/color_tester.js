window.addEventListener("load", function (e) {
    function updateColorCanvas() {
        var canvas = document.getElementById("color_canvas");
        var h_value = document.getElementById("h_value");
        var a_value = document.getElementById("a_value");
        var b_value = document.getElementById("b_value");
        var c_value = document.getElementById("c_value");
        var line_color = document.getElementById("line_color");

        var ctx = canvas.getContext("2d");
        var canvasData = ctx.createImageData(canvas.width, canvas.height);
        
        for (var y = 0; y < canvasData.height; ++y) {
            for (var x = 0; x < canvasData.width; ++x) {
                // HSV to RGB
                // http://www.rapidtables.com/convert/color/hsv-to-rgb.htm
                var h = h_value.value;
                var s = x / canvasData.width;
                var v = (canvasData.height - y) / canvasData.height;

                var tc = v * s;
                var tx = tc * (1 - Math.abs((h / 60) % 2 - 1));
                var tm = v - tc;
                var rp;
                var gp;
                var bp;
                if (h >= 0 && h < 60) {
                    rp = tc;
                    gp = tx;
                    bp = 0;
                }
                else if (h >= 60 && h < 120) {
                    rp = tx;
                    gp = tc;
                    bp = 0;
                }
                else if (h >= 120 && h < 180) {
                    rp = 0;
                    gp = tc;
                    bp = tx;
                }
                else if (h >= 180 && h < 240) {
                    rp = 0;
                    gp = tx;
                    bp = tc;
                }
                else if (h >= 240 && h < 300) {
                    rp = tx;
                    gp = 0;
                    bp = tc;
                }
                else {
                    rp = tc;
                    gp = 0;
                    bp = tx;
                }
                var r = (rp + tm) * 255;
                var g = (gp + tm) * 255;
                var b = (bp + tm) * 255;

                var idx = (y * canvasData.width + x) * 4;
                canvasData.data[idx + 0] = r; // Red channel
                canvasData.data[idx + 1] = g; // Green channel
                canvasData.data[idx + 2] = b; // Blue channel
                canvasData.data[idx + 3] = 0xff; // Alpha channel
            }
        }

        for (var x = 0; x < canvasData.width; ++x) {
            var a = parseFloat(a_value.value);
            var b = parseFloat(b_value.value);
            var c = parseFloat(c_value.value);
            var xp = x / canvasData.width;

            if (xp - b != 0) {
                var yp = (a / (xp - b)) + c;

                if (yp >= 0 && yp < 1) {
                    var y = parseInt(canvasData.height - yp * canvasData.height);
                    var rgb = hexToRgb(line_color.value);
                    var idx = (y * canvasData.width + x) * 4;
                    
                    canvasData.data[idx + 0] = rgb.r; // Red channel
                    canvasData.data[idx + 1] = rgb.g; // Green channel
                    canvasData.data[idx + 2] = rgb.b; // Blue channel
                    canvasData.data[idx + 3] = 0xff; // Alpha channel
                }
            }
        }
        for (var y = 0; y < canvasData.height; ++y) {
            var a = parseFloat(a_value.value);
            var b = parseFloat(b_value.value);
            var c = parseFloat(c_value.value);
            var yp = (canvasData.height - y) / canvasData.height;

            if (yp - c != 0) {
                var xp = a / (yp - c) + b;

                if (xp >= 0 && xp < 1) {
                    var x = parseInt(xp * canvasData.width);
                    var rgb = hexToRgb(line_color.value);
                    var idx = (y * canvasData.width + x) * 4;
                    
                    canvasData.data[idx + 0] = rgb.r; // Red channel
                    canvasData.data[idx + 1] = rgb.g; // Green channel
                    canvasData.data[idx + 2] = rgb.b; // Blue channel
                    canvasData.data[idx + 3] = 0xff; // Alpha channel
                }
            }
        }

        ctx.putImageData(canvasData, 0, 0);
    }

    function hexToRgb(hex) {
        // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
        var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
        hex = hex.replace(shorthandRegex, function(m, r, g, b) {
            return r + r + g + g + b + b;
        });

        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        return result ? {
            r: parseInt(result[1], 16),
            g: parseInt(result[2], 16),
            b: parseInt(result[3], 16)
        } : null;
    }

    function initialize() {
        var h_value = document.getElementById("h_value");
        var a_value = document.getElementById("a_value");
        var b_value = document.getElementById("b_value");
        var c_value = document.getElementById("c_value");
        var line_color = document.getElementById("line_color");

        h_value.addEventListener("change", updateColorCanvas);
        a_value.addEventListener("change", updateColorCanvas);
        b_value.addEventListener("change", updateColorCanvas);
        c_value.addEventListener("change", updateColorCanvas);
        line_color.addEventListener("change", updateColorCanvas);

        updateColorCanvas();
    }

    initialize();

});
