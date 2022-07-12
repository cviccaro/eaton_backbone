<?php

module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/RotateableMemImageFpdf.class');
module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/MemImageFpdf.class');

module_load_include('inc', 'eaton_backbone', 'inc/StickerSheet.class');

/**
 * The number of rows in this game sheet.
 */
define('BGSS_NUM_ROWS', 6);

/**
 * The number of columns in this game sheet.
 */
define('BGSS_NUM_COLS', 6);

/**
 * @file
 * Sticker sheet layout for Barcode Graphics (www.barcode-us.com) BC-36
 * 1.625" x 1.25" per sticker, 36-sticker (6x6) sticker stock.
 *
 * @author
 * Guy Paddock (gapopl@rit.edu)
 */
class BarcodeGraphicsStickerSheet extends StickerSheet {
  /**
   * Initialize a new Barcode Graphics sticker sheet with the provided sticker content.
   *
   * @param $stickers
   *   All the stickers on this sheet.
   */
  public function BarcodeGraphicsStickerSheet($numRows, $numCols, $stickers = NULL) {
    parent::__construct($numRows, $numCols , $stickers);
  }

  /**
   * Render this gamesheet to the specified PDF writer.
   *
   * @param $pdf
   *   The FPDF instance to use for output.
   */
  public function renderTo(RotateableMemImageFpdf $pdf) {
    $pdf->AddPage();

    if ($this->headerText != NULL) {
      // Header text on left side
      $this->addLeftSideText($pdf);
    }

    if ($this->footerText != NULL) {
      // Footer text on right side
      $this->addRightSideText($pdf);
    }

    // Lay out the barcodes
    $this->addBarcodes($pdf);
  }

  /**
   * Add the text to the left side of the gamesheet.
   * 
   * @param $pdf
   *   The FPDF instance to use for output.
   */
  protected function addLeftSideText(RotateableMemImageFpdf $pdf) {
    $pdf->SetTextColor(0, 0, 0);
    $pdf->SetFont('Helvetica', '', 10);

    $convertedText = utf8_decode($this->headerText);

    $headerWidth   = $pdf->GetStringWidth($convertedText);
    $headerX       = 0.35;
    $headerY       = (11 / 2) + ($headerWidth / 2);

    $pdf->RotatedText($headerX, $headerY, $convertedText, 90);
  }

  /**
   * Add the text to the right side of the gamesheet.
   *
   * @param $pdf
   *   The FPDF instance to use for output.
   */
  protected function addRightSideText(RotateableMemImageFpdf $pdf) {
    $pdf->SetTextColor(0, 0, 0);
    $pdf->SetFont('Helvetica', '', 10);

    $convertedText = utf8_decode($this->footerText);

    $footerWidth   = $pdf->GetStringWidth($convertedText);
    $footerX       = 8.5 - 0.25;
    $footerY       = (11 / 2) + ($footerWidth / 2);

    $pdf->RotatedText($footerX, $footerY, $convertedText, 90);
  }

  /**
   * Add the barcodes to the gamesheet.
   *
   * @param $pdf
   *   The FPDF instance to use for output.
   */
  protected function addBarcodes(MemImageFpdf $pdf) {
    // Margin size in inches.
    $topMargin    = 0.25;
    $sideMargin   = 0.5;

    $padding      = 0.05;

    $horizSpacing = 0;
    $vertSpacing  = 0.15;

    $totalHorizSpacing = $horizSpacing * ($this->numColumns - 1);
    $totalVertSpacing  = $vertSpacing  * ($this->numRows - 1);

    // Size in inches.
    $stickerWidth  = (8.5 - (2 * $sideMargin) - $totalHorizSpacing) / $this->numColumns;
    $stickerHeight = (11  - (2 * $topMargin)  - $totalVertSpacing) / $this->numRows;

    $barCodeWidth  = $stickerWidth  - (2 * $padding);

    for ($row = 0; $row < $this->numRows; ++$row) {
      for ($col = 0; $col < $this->numColumns; ++$col) {
        $sticker = $this->stickers[$row][$col];

        // Allow for empty stickers.
        if (isset($sticker)) {
          $startX = $sideMargin + (($stickerWidth  + $horizSpacing) * $col) + $padding;
          $startY = $topMargin  + (($stickerHeight + $vertSpacing)  * $row) + $padding;

          // height is 0 to allow auto-scaling.
          $sticker->render($pdf, $startX, $startY, $barCodeWidth, 0);
        }
      }
    }
  }
}
