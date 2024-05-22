# Custom Libraries

To reduce the complexity of the code of the main project file, our team created some own libraries to achieve a better organized code.

## Layout Librarie

The layout librarie was made to provide some functions to show content in the oled screen based in the default organization on the screen. The default organization is based on our project needs, according to the image below.
[Indexed Layout Image](assets/Layout-lib-indexed.png)

-   Blue squares represents icons and logo;
-   Green square represents timer;
-   Red lines represents text rows;
    Based on this layout, the functions of the librarie were made to fill our needs.

### Functions

**Layout(int screen_width, int screen_height, int reset_pin):** Initialize layout object;
**beginLayout():** Draw the basic layout in the screen;
**beginTimer():** Draw an empty timer in the timer square;
**updateTimer(int counter, byte interval):** Receives an counter and an byte containing seconds interval to calculate the time to be shown;
**drawLogo(const unsigned char \*logo):** Draw an 16x16 bitmap in the logo square;
**drawIcon(int index, const unsigned char \*logo):** Receive an index (organization in the image above) and an 8x8 bitmap to show the icon in the respective square;
**eraseIcon(int index):** Receive an index (organization in the image above) and erases the icon in the respective square;
**writeLine(int index, String text):** Receive an index (organization in the image above) and write the text provided in the respective row/line.

## Icons Librarie

That's an simple librarie to return the respective icon based in the function called. There are some 8x8 bitmap images and a bigger 16x16 bitmap image for logo.

### Functions (Icons)

**sparcLogo();**
**loadingIcon();**
**failedIcon();**
**successIcon();**
**keyIcon();**
**wifiIcon();**
**waterIcon();**
**wrenchIcon();**
You only need to provide the function to get the icon.
