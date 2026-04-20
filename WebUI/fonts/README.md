# Flowstate Plugin Fonts

## Required Font Files

The Flowstate plugin UI requires the Inter font family to be placed in this directory.

### Required Files:
- `Inter-Regular.woff2` (400 weight)
- `Inter-Medium.woff2` (500 weight)
- `Inter-SemiBold.woff2` (600 weight)

### Download Instructions:

1. Visit the Inter font website: https://rsms.me/inter/
2. Download the Inter font family
3. Extract the `.woff2` files for Regular, Medium, and SemiBold weights
4. Place them in this `WebUI/fonts/` directory

### Alternative:

You can also use Google Fonts or another CDN, but for offline operation (as required by the spec), local font files are necessary.

### License:

Inter is licensed under the SIL Open Font License 1.1
https://github.com/rsms/inter/blob/master/LICENSE.txt

## Current Status

⚠️ **Font files are not included in the repository**

The UI will fall back to system fonts (-apple-system, BlinkMacSystemFont) if Inter fonts are not available, but for the best visual experience, please add the Inter font files as described above.
