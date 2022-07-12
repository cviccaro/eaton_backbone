<?php

module_load_include('php', 'eaton_backbone', 'inc/lib/fpdf/fpdf');
module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/MemImageFpdf.class');

module_load_include('inc', 'eaton_backbone', 'inc/Sticker.class');

/**
 * The space between a barcode and its text.
 *
 * TODO: Make this configurable?
 *
 * @var int
 */
define("BARCODE_TEXT_MARGIN", 0);

/**
 * @file
 * A single QR-code sticker.
 *
 * @author
 * Guy Paddock (gapopl@rit.edu)
 */
class QrCodeSticker extends Sticker {
  /**
   * The pre-generated QR-code image; saved here so we don't have to
   * generate it each time we render.
   *
   * @var resource
   */
  private $barcodeImage;

  /**
   * The width of the barcode image, in pixels.
   *
   * @var int
   */
  private $barcodeWidth;

  /**
   * The height of the barcode image, in pixels.
   *
   * @var int
   */
  private $barcodeHeight;

  /**
   * The content that will be encoded in the barcode image of this sticker.
   *
   * @var string
   */
  private $barcodeContent;

  /**
   * The text that will be printed in the label below the barcode of this sticker.
   *
   * @var string
   */
  private $labelText;

  /**
   * Initialize a new QrCodeSticker instance with the specified content and label text.
   *
   * @param $barcodeContent
   *   The content that will be encoded in the barcode image of this sticker.
   *
   * @param $labelText
   *   The text that will be printed in the label below the barcode of this sticker.
   */
  public function QrCodeSticker($barcodeContent = NULL, $labelText = NULL) {
    $this->setBarcodeContent($barcodeContent);
    $this->setLabelText($labelText);
  }

  /**
   * Destructor for QrCodeSticker.
   */
  public function __destruct() {
    if ($this->barcodeImage != NULL) {
      imagedestroy($this->barcodeImage);

      $this->barcodeImage = NULL;
    }
  }

  /**
   * Set the content that will be encoded in the barcode image of this sticker.
   *
   * @param $content
   *   The content that will be encoded in the barcode image of this sticker.
   */
  public function setBarcodeContent($barcodeContent) {
    $this->barcodeContent  = $barcodeContent;

    if ($barcodeContent != NULL) {
      $this->barcodeImage  = $this->generateBarcode();
      $this->barcodeWidth  = imagesx($this->barcodeImage);
      $this->barcodeHeight = imagesy($this->barcodeImage);
    }
  }

  /**
   * Set the text that will be printed in the label below the barcode of this sticker.
   *
   * @param $label
   *   The text that will be printed in the label below the barcode of this sticker.
   */
  public function setLabelText($label) {
    $this->labelText = utf8_decode($label);
  }

  /**
   * Render this sticker to the provided PDF renderer at the position and size specified.
   *
   * If height is 0, the sticker will be scaled appropriately by the width to maintain the appropriate aspect
   * ratio. The same is not true if width is 0.
   *
   * @param $pdf
   *   The FPDF writer to use for output.
   *
   * @param $x
   *   The x position of the upper-left corner of the
   *   sticker.
   *
   * @param $y
   *   The y position of the upper-left corner of the
   *   sticker.
   *
   * @param $width
   *   The width of the sticker.
   *
   * @param $height
   *   The height of the sticker.
   *
   * @throws RuntimeException
   *   If the destructor has been invoked on this object before calling this method.
   */
  public function render(MemImageFpdf $pdf, $x, $y, $width, $height) {
    if (($width == 0) && ($height == 0)) {
      throw new InvalidArgumentException('$width and $height cannot both be 0.');
    }

    // Needed for later math.
    $pdf->SetFont('Courier', 'B', 10);

    $imageWidth  = 0;
    $imageHeight = 0;

    if ($this->barcodeContent != NULL) {
      if ($this->barcodeImage == NULL) {
        throw new RuntimeException(
            'Barcode image is NULL. Ensure that the destructor has not been invoked on this '.
            'instance.');
      }

      if ($width == 0) {
        // Calculate width from height.
        $this->renderImageWithImplicitWidth($pdf, $x, $y, $height, $imageWidth, $imageHeight);
      }

      elseif ($height == 0) {
        // Calculate height from width.
        // $imageHeight = $imageHeight / .66;
        // $imageWidth = $imageWidth / .66;
        $this->renderImageWithImplicitHeight($pdf, $x, $y, $width, $imageWidth, $imageHeight);
      }

      else {
        // Force width & height.
        $this->renderImageWithFixedSize($pdf, $x, $y, $width, $height, $imageWidth, $imageHeight);
      }
    }

    else {
      // Center the label on the sticker.
      $imageWidth = $width;
    }
    if ($this->labelText != NULL) {
      $this->renderLabelText($pdf, $x, $y, $imageWidth, $imageHeight);
    }
  }

  /**
   * Render the QR code image with a width that is calculated implicitly based on the provided height and the
   * aspect-ratio of the QR code itself, while leaving space for the text at the bottom of the sticker.
   *
   * @param $pdf
   *   The FPDF writer to use for output.
   *
   * @param $stickerX
   *   The x position of the upper-left corner of the
   *   sticker.
   *
   * @param $stickerY
   *   The y position of the upper-left corner of the
   *   sticker.
   *
   * @param $height
   *   The fixed height that the full sticker will
   *   need to fill.
   *
   * @param $imageWidth
   *   Output parameter that provides the final
   *   width of the QR code image, in user units.
   *
   * @param $imageHeight
   *   Output parameter that provides the final
   *   height of the QR code image, in user units. This
   *   will differ from $height because the space
   *   needed for the text at the bottom of the sticker
   *   is taken into account.
   */
  protected function renderImageWithImplicitWidth(MemImageFpdf $pdf,
      $stickerX,
      $stickerY,
      $height,
      &$imageWidth,
      &$imageHeight) {
    $lineHeight  = $pdf->FontSizePt * 1.5;

    $imageHeight = $height - $lineHeight - BARCODE_TEXT_MARGIN;
    $imageWidth  = ($this->barcodeWidth / $this->barcodeHeight) * $imageHeight;

    $pdf->GDImage($this->barcodeImage, $stickerX, $stickerY, $imageWidth, $imageHeight);
  }

  /**
   * Render the QR code image with a height that is calculated implicitly
   * based on the provided width and the aspect-ratio of the QR code
   * itself.
   *
   * @param $pdf
   *   The FPDF writer to use for output.
   *
   * @param $stickerX
   *   The x position of the upper-left corner of the
   *   sticker.
   *
   * @param $stickerY
   *   The y position of the upper-left corner of the
   *   sticker.
   *
   * @param $width
   *   The fixed width that the full sticker will
   *   need to fill.
   *
   * @param $imageWidth
   *   Output parameter that provides the final
   *   width of the QR code image, in user units.
   *
   * @param $imageHeight
   *   Output parameter that provides the final
   *   height of the QR code image, in user units.
   */
  protected function renderImageWithImplicitHeight(MemImageFpdf $pdf,
      $stickerX,
      $stickerY,
      $width,
      &$imageWidth,
      &$imageHeight) {
    $imageWidth  = $width;
    $imageHeight = ($this->barcodeHeight / $this->barcodeWidth) * $imageWidth;
    
    // $imageWidth = $imageWidth * .96;
    // $imageHeight = $imageHeight * .96;
    $pdf->GDImage($this->barcodeImage, $stickerX, $stickerY, $imageWidth, $imageHeight);
  }

  /**
   * Render the QR code image with the provided fixed width and height,
   * leaving space for the text at the bottom of the sticker.
   *
   * @param $pdf
   *   The FPDF writer to use for output.
   *
   * @param $stickerX
   *   The x position of the upper-left corner of the
   *   sticker.
   *
   * @param $stickerY
   *   The y position of the upper-left corner of the
   *   sticker.
   *
   * @param $width
   *   The fixed width that the full sticker will
   *   need to fill.
   *
   * @param $height
   *   The fixed height that the full sticker will
   *   need to fill.
   *
   * @param $imageWidth
   *   Output parameter that provides the final
   *   width of the QR code image, in user units.
   *
   * @param $imageHeight
   *   Output parameter that provides the final
   *   height of the QR code image, in user units. This
   *   will differ from $height because the space
   *   needed for the text at the bottom of the sticker
   *   is taken into account.
   */
  protected function renderImageWithFixedSize(MemImageFpdf $pdf,
      $stickerX, $stickerY,
      $width, $height,
      &$imageWidth, &$imageHeight) {
    $lineHeight  = $pdf->FontSizePt * 1.5;

    $imageHeight = $height - $lineHeight + BARCODE_TEXT_MARGIN;
    $imageWidth  = $width;

    $pdf->GDImage($this->barcodeImage, $stickerX, $stickerY, $imageWidth, $imageHeight);
  }

  /**
   * Render the text at the bottom of the sticker.
   *
   * @param $pdf
   *   The FPDF writer to use for output.
   *
   * @param $stickerX
   *   The x position of the upper-left corner of the
   *   sticker.
   *
   * @param $stickerY
   *   The y position of the upper-left corner of the
   *   sticker.
   *
   * @param $imageWidth
   *   The width of the image above the text.
   *
   * @param $imageHeight
   *   The height of the image above the text.
   */
  protected function renderLabelText(FPDF $pdf, $stickerX, $stickerY, $imageWidth, $imageHeight) {
    $textY = $stickerY + $imageHeight + BARCODE_TEXT_MARGIN - .1;

    $convertedText = utf8_decode($this->labelText);

    // Split into lines.
    $labelLines = preg_split('/\n/', $convertedText);

    foreach ($labelLines as $line) {
      $trimLine  = trim($line);
      $textWidth = $pdf->GetStringWidth($trimLine);
      $textX     = $stickerX + (($imageWidth - $textWidth) / 2);

      $pdf->Text($textX, $textY, $trimLine);

      // Move down a line.
      $textY += $pdf->FontSize * 1.1;
      //watchdog('qrcodesticker','placing label ' . $line . ' at ' . $textX . 'x' . $textY,NULL,WATCHDOG_INFO);
    }
  }

  /**
   * Use the QR_IMG library to generate the QR barcode.
   *
   * @return
   *   The image resource containing the barcode.
   */
  protected function generateBarcode() {
    /**
     * We run the qr_img script like it would be run if it had received an HTTP
     * GET request containing the parameters for the QR-code, grab the
     * resulting image resource, and throw away the data that the script would
     * have normally sent back to the HTTP client.
     *
     * Yes, this is a hack. What of it?
     */
    global $output_image;

    $_GET['d'] = $this->barcodeContent;  // Data is the barcode content
    $_GET['s'] = 5;                      // Size 8
    $_GET['v'] = 0;                      // Version 15

    ob_start();

    /*
     * NOTE: This will put a content-type header into the headers list.
     *       It's okay, though, because it will get replaced by the PDF
     *       generation code.
     */
    include('./' . drupal_get_path('module', 'eaton_backbone') . '/inc/lib/qr_img/qr_img.php');
    if (!headers_sent()) {
      header('Content-Type: text/html');
    }

    // Throw away the PNG output -- we only care about the image that was
    // generated.
    ob_end_clean();

    return $output_image;
  }
  public function genBarcode() {
     global $output_image;

    $_GET['d'] = $this->barcodeContent;  // Data is the barcode content
    $_GET['s'] = 5;                      // Size 8
    $_GET['v'] = 0;                      // Version 15

    ob_start();
    
    include('./' . drupal_get_path('module', 'eaton_backbone') . '/inc/lib/qr_img/qr_img.php');
    $contents = ob_get_contents();
    ob_end_clean();
    return $contents;
  }
}
