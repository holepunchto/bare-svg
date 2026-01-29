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

test('encode throws', (t) => {
  t.exception(() => svg.encode(Buffer.alloc(0)))
})

test('encodeAnimated throws', (t) => {
  t.exception(() => svg.encodeAnimated([]))
})
