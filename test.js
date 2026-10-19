const test = require('brittle')
const svg = require('.')

const fixture = require('./test/fixtures/sample.svg', {
  with: { type: 'text' }
})

test('decode', (t) => {
  const result = svg.decode(fixture)

  t.ok(result.width > 0)
  t.ok(result.height > 0)
  t.ok(Buffer.isBuffer(result.data))
})

test('decode with width option', (t) => {
  const result = svg.decode(fixture, { width: 100 })

  t.ok(result.width > 0)
  t.ok(result.height > 0)
  t.ok(Buffer.isBuffer(result.data))
})

test('decode with height option', (t) => {
  const result = svg.decode(fixture, { height: 100 })

  t.ok(result.width > 0)
  t.ok(result.height > 0)
  t.ok(Buffer.isBuffer(result.data))
})

test('decode with loadFonts false', (t) => {
  const result = svg.decode(fixture, { loadFonts: false })

  t.ok(result.width > 0)
  t.ok(result.height > 0)
  t.ok(Buffer.isBuffer(result.data))
})

test('decode accepts a Buffer', (t) => {
  const result = svg.decode(Buffer.from(fixture))

  t.ok(result.width > 0)
  t.ok(result.height > 0)
  t.ok(Buffer.isBuffer(result.data))
})

test('decode throws on non-object options', (t) => {
  t.exception.all(() => svg.decode(fixture, 42), /Options must be an object/)
})

test('decode throws on a canvas that rounds to zero pixels', (t) => {
  t.exception(
    () => svg.decode('<svg xmlns="http://www.w3.org/2000/svg" width="48" height=".2"></svg>'),
    /dimensions are out of range/
  )
})

test('decode throws on a canvas beyond the pixel cap', (t) => {
  t.exception(
    () =>
      svg.decode('<svg xmlns="http://www.w3.org/2000/svg" width="100000" height="100000"></svg>'),
    /dimensions are out of range/
  )
})

test('decode rounds a sub-pixel dimension up', (t) => {
  const result = svg.decode('<svg xmlns="http://www.w3.org/2000/svg" width="48" height=".7"></svg>')

  t.is(result.width, 48)
  t.is(result.height, 1)
})

test('decode throws on a width option that rounds to zero pixels', (t) => {
  t.exception(() => svg.decode(fixture, { width: 0.2 }), /dimensions are out of range/)
})

test('decode throws on malformed SVG', (t) => {
  t.exception(() => svg.decode('this is not an svg document'))
})

test('decode throws when parsing panics', (t) => {
  // A nested SVG image whose size overflows to infinity panics inside usvg.
  const image = `<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64"><image width="10" height="10" href="data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' width='1e38' height='1e38'%3E%3Crect width='1' height='1'/%3E%3C/svg%3E"/></svg>`

  t.exception(() => svg.decode(image), /Failed to parse SVG/)
})

test('decode throws when rendering panics', (t) => {
  // An offset that overflows the filter region panics inside tiny-skia.
  const image = `<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64"><filter id="f"><feOffset dx="1e38" dy="1e38"/></filter><rect width="10" height="10" filter="url(#f)"/></svg>`

  t.exception(() => svg.decode(image), /Failed to render SVG/)
})

test('encode throws', (t) => {
  t.exception(() => svg.encode(Buffer.alloc(0)))
})

test('encodeAnimated throws', (t) => {
  t.exception(() => svg.encodeAnimated([]))
})
