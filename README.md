# bare-svg

SVG support for Bare.

```
npm i bare-svg
```

## Usage

```js
const svg = require('bare-svg')

const image = require('./my-image.svg', { with: { type: 'text' } })

const decoded = svg.decode(image, {
  width: 100, // optional, default from SVG or 512
  height: 200, // optional, default from SVG or 512
  dpi: 96, // optional, default 96
  loadFonts: false // optional, default true, false is faster
})
// {
//   width: 100,
//   height: 200,
//   data: <Buffer>
// }
```

## License

Apache-2.0
