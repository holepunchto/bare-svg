const binding = require('./binding')

exports.decode = function decode(svg, opts = {}) {
  const result = binding.decode(svg, opts)

  return {
    width: result.width,
    height: result.height,
    data: Buffer.from(result.data)
  }
}

exports.encode = function encode(rgba, opts = {}) {
  throw new Error('SVG encoding not supported')
}

exports.encodeAnimated = function encodeAnimated(frames, opts = {}) {
  throw new Error('Animated SVG not supported')
}
