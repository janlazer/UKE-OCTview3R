# Rebuild runtime PNG and Windows multi-resolution icon from the selected logo.
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$assetDirectory = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\OCTview3R\Resources\branding'))
$source = [Drawing.Bitmap]::new((Join-Path $assetDirectory 'octview3r-logo.png'))

function Get-LogoPng([int]$size) {
    $bitmap = [Drawing.Bitmap]::new($size, $size, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    $stream = [IO.MemoryStream]::new()
    try {
        $graphics.CompositingMode = [Drawing.Drawing2D.CompositingMode]::SourceCopy
        $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graphics.PixelOffsetMode = [Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $graphics.DrawImage($source, [Drawing.Rectangle]::new(0, 0, $size, $size))
        $bitmap.Save($stream, [Drawing.Imaging.ImageFormat]::Png)
        return ,$stream.ToArray()
    } finally {
        $stream.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

try {
    [IO.File]::WriteAllBytes((Join-Path $assetDirectory 'octview3r-ui.png'), (Get-LogoPng 512))
    $sizes = @(16, 20, 24, 32, 40, 48, 64, 128, 256)
    $images = @($sizes | ForEach-Object { ,(Get-LogoPng $_) })
    $iconStream = [IO.File]::Create((Join-Path $assetDirectory 'octview3r.ico'))
    $writer = [IO.BinaryWriter]::new($iconStream)
    try {
        $writer.Write([uint16]0)
        $writer.Write([uint16]1)
        $writer.Write([uint16]$sizes.Count)
        $offset = 6 + 16 * $sizes.Count
        for ($i = 0; $i -lt $sizes.Count; $i++) {
            $dimension = if ($sizes[$i] -eq 256) { 0 } else { $sizes[$i] }
            $writer.Write([byte]$dimension)
            $writer.Write([byte]$dimension)
            $writer.Write([byte]0)
            $writer.Write([byte]0)
            $writer.Write([uint16]1)
            $writer.Write([uint16]32)
            $writer.Write([uint32]$images[$i].Length)
            $writer.Write([uint32]$offset)
            $offset += $images[$i].Length
        }
        foreach ($image in $images) { $writer.Write([byte[]]$image) }
    } finally {
        $writer.Dispose()
    }
    Write-Output "Created transparent 512 px UI logo and ICO sizes: $($sizes -join ', ')."
} finally {
    $source.Dispose()
}
