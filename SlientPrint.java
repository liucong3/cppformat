
/*
 * ===========================================
 * Java Pdf Extraction Decoding Access Library
 * ===========================================
 *
 * Project Info:  http://www.idrsolutions.com
 * Help section for developers at http://www.idrsolutions.com/support/
 *
 * (C) Copyright 1997-2016 IDRsolutions and Contributors.
 *
 * This file is part of JPedal/JPDF2HTML5
 *
 
 *
 * ---------------
 * SilentPrint.java
 * ---------------
 */

package org.jpedal.examples.printing;

import java.awt.print.*;
import java.io.File;

import javax.print.*;
import javax.print.attribute.Attribute;
import javax.print.attribute.HashPrintRequestAttributeSet;
import javax.print.attribute.PrintRequestAttributeSet;
import javax.print.attribute.standard.Copies;
import javax.print.attribute.standard.JobName;
import javax.print.attribute.standard.PageRanges;
import javax.print.attribute.standard.SheetCollate;
import javax.print.event.PrintJobEvent;
import javax.print.event.PrintJobListener;

import org.jpedal.PdfDecoder;
import org.jpedal.fonts.FontMappings;
import org.jpedal.objects.PrinterOptions;
import org.jpedal.utils.LogWriter;
import org.jpedal.utils.PdfBook;

/**
 * <h2><b>This Example Prints a PDF File or Directory of Files.</b></h2>
 *
 * <p>It can run from jar directly using the command:
 *
 * <br><b>java -cp libraries_needed org/jpedal/examples/printing/SilentPrint inputValues</b></p>
 *
 * <p>Where inputValues is parameters (file or directory and name of printer). If printer is invalid, a message will be returned with a list of values which can be used.</p>
 * 
 * <p><a href="http://www.idrsolutions.com/support/java-pdf-library-support/how-to-print-pdf-files-in-java/">See our support pages for more information on printing with JPedal.</a></p>
 * 
 * @deprecated  As of release 6.2, replaced by PrintPdfPage
 */
@Deprecated
public class SilentPrint {


    /** correct separator for OS */
    private final String separator = System.getProperty("file.separator");

    /** the decoder object which decodes the pdf and returns a data object */
    private PdfDecoder pdfDecoder;

    /** number of page to be printed */
    public static final int pageMark = 1;

    /** needs to be global for the printer selection to work */
    public static DocPrintJob printJob;

    /**
     * example method to open a file and print the pages
     */
    public SilentPrint(String file_name, final String printerName) {

        //Ensure printer is valid
        final boolean validPrinter = validatePrinter(printerName);

        if(validPrinter){
            /**
             * if file name ends pdf, do the file otherwise do every pdf file in the
             * directory. We already know file or directory exists so no need to
             * check that, but we do need to check its a directory
             */
            if (file_name.toLowerCase().endsWith(".pdf")) {
                decodeAndPrintFile(file_name);
            }
            else {

                /**
                 * get list of files and check directory
                 */
                String[] files = null;
                final File inputFiles;

                /** make sure name ends with a deliminator for correct path later */
                if (!file_name.endsWith(separator)) {
                    file_name += separator;
                }

                try {
                    inputFiles = new File(file_name);

                    if (!inputFiles.isDirectory()) {
                        System.err.println(file_name + " is not a directory. Exiting program");
                    } else {
                        files = inputFiles.list();
                    }
                }
                catch (final Exception ee) {
                    LogWriter.writeLog("Exception trying to access file " + ee.getMessage());
                }

                /** now work through all pdf files */
                for (final String file : files) {

                    if (file.toLowerCase().endsWith(".pdf")) {
                        logMessage(file_name + file);

                        decodeAndPrintFile(file_name + file);

                        // reset printer in case of lots of pages (cannot be reused)
                        validatePrinter(printerName);

                    }
                }
            }
        }else{
            //The printer is not valid
            logMessage("FAILED TO IDENTIFY PRINTER");
        }
    }

    /**
     * routine to decode a file and print it
     */
    private void decodeAndPrintFile(final String file_name) {

        /**
         * open the file and get page count
         */
        try {

            logMessage("Opening file :" + file_name + " to print.");

            pdfDecoder = new PdfDecoder(true);
            // decode_pdf.setExtractionMode(0, 72,1);
            pdfDecoder.openPdfFile(file_name);

        } catch (final Exception e) {
            reportError("Exception " + e + " in pdf code");
        }

        /**
         * code to handle passwords - some PDFs have an empty password set
         */
        if (pdfDecoder.isEncrypted() && !pdfDecoder.isFileViewable()) {

            final String password = null; // set your password here

            try {
                /** try and reopen with new password */
                if (password == null) {
                    pdfDecoder.setEncryptionPassword(password);
                } else {
                    pdfDecoder.setEncryptionPassword("");
                }

            } catch (final Exception e) {
                e.printStackTrace();
            }
        }

        /**
         * print if allowed and values found
         */
        if (pdfDecoder.isEncrypted() && !pdfDecoder.isFileViewable()) {
            logMessage("Encrypted settings");
        } else {

            // mappings for non-embedded fonts to use
            FontMappings.setFontReplacements();

            printPages();
        }

        /** close the pdf file */
        pdfDecoder.closePdfFile();

    }

    /**
     * PRINTING CODE EXAMPLE.  If you put this into a thread you will need to synchronize
     * and ensure terminated if program exits.
     *
     * Uses pageable interface so does not work under 1.3
     */
    private void printPages()
    {
        //Create set of attributes
        final PrintRequestAttributeSet attributes = getPrintRequestAttributeSet();

        /**
         * workaround to improve performance on PCL printing by printing
         * using drawString or Java's glyph if font available in Java
         */
        // decode_pdf.setTextPrint(PdfDecoder.NOTEXTPRINT); //normal mode - only needed to reset
        // decode_pdf.setTextPrint(PdfDecoder.STANDARDTEXTSTRINGPRINT);  //print all standard fonts with Java regardless of if embedded
        // decode_pdf.setTextPrint(PdfDecoder.TEXTGLYPHPRINT); //intermediate mode - let Java create Glyphs if font matches
        // decode_pdf.setTextPrint(PdfDecoder.TEXTSTRINGPRINT); //try and get Java to do all the work

        //Automatically rotate and center the Pdf pages for best fit.  Default is true.
        pdfDecoder.setPrintAutoRotateAndCenter(true);

        //Pdf page scaling options
//    pdfDecoder.setPrintPageScalingMode(PrinterOptions.PAGE_SCALING_FIT_TO_PRINTER_MARGINS);
//    pdfDecoder.setPrintPageScalingMode(PrinterOptions.PAGE_SCALING_NONE);
        pdfDecoder.setPrintPageScalingMode(PrinterOptions.PAGE_SCALING_REDUCE_TO_PRINTER_MARGINS);

        final PdfBook pdfBook = new PdfBook(pdfDecoder, printJob.getPrintService(), attributes);
        pdfBook.setChooseSourceByPdfPageSize(false);

        final SimpleDoc doc = new SimpleDoc(pdfBook, DocFlavor.SERVICE_FORMATTED.PAGEABLE, null);

        // used to track print activity
        printJob.addPrintJobListener(new PDFPrintJobListener());

        try {
            printJob.print(doc, attributes);
        }
        catch (final Exception e) {
            LogWriter.writeLog("Exception " + e + " printing");
        }

        // any errors reported in JPedal
        if (!pdfDecoder.getPageFailureMessage().isEmpty()) {
            System.out.println("Errors reported from JPedal=" + pdfDecoder.getPageFailureMessage());
        }

    }

    /**
     * This example method sets up a few attributes for a PrintService.
     *
     * See http://download.oracle.com/javase/1.4.2/docs/api/javax/print/attribute/PrintRequestAttribute.html
     * http://download.oracle.com/javase/tutorial/2d/printing/services.html and
     * http://download.oracle.com/javase/7/docs/technotes/guides/jps/spec/attributes.fm.html
     * for further information regarding attributes.
     */
    private PrintRequestAttributeSet getPrintRequestAttributeSet()
    {
        final PrintRequestAttributeSet attributeSet = new HashPrintRequestAttributeSet();

        // Name the print job
        final String[] jobString = pdfDecoder.getFileName().split("/");
        final JobName jobName = new JobName(jobString[jobString.length-1], null);

        if(validateAttribute(jobName, attributeSet)) {
            attributeSet.add(jobName);
        }

        //Print multiple copies
        if(pageMark > 1) {
            final Copies copies = new Copies(pageMark);
            final SheetCollate collate = SheetCollate.COLLATED;
            if(validateAttribute(copies, attributeSet) && validateAttribute(collate, attributeSet)) {
                attributeSet.add(copies);
                attributeSet.add(collate);
            }
        }

        //Print out a page range
        final PageRanges range = new PageRanges("1-10");
        if(validateAttribute(range, attributeSet)) {
            attributeSet.add(range);
        }

        // Black and white print
//        if(validateAttribute(Chromaticity.MONOCHROME, attributeSet)) {
//          attributeSet.add(Chromaticity.MONOCHROME);
//        }

        // Draft print quality
//        if(validateAttribute(PrintQuality.DRAFT, attributeSet)) {
//          attributeSet.add(PrintQuality.DRAFT);
//        }

        return attributeSet;
    }

    /**
     * @return true if the attribute is supported on the current PrintService
     */
    private static boolean validateAttribute(final Attribute att, final PrintRequestAttributeSet attributeSet)
    {
        return printJob.getPrintService().isAttributeValueSupported(att, DocFlavor.SERVICE_FORMATTED.PAGEABLE, attributeSet);
    }

    /**
     * Checks that the desired PrintService exists and sets it as the PrintService to use
     *
     * @param newPrinter The name of the PrintService
     * @return true if PrintService is available
     * @throws PrinterException
     */
    private static boolean validatePrinter(final String newPrinter) {

        boolean matchFound = false;

        final PrintService[] service = PrinterJob.lookupPrintServices(); // list of printers

        final int count = service.length;

        for (int i = 0; i < count; i++) {
            if (service[i].getName().contains(newPrinter)) {
                printJob = service[i].createPrintJob();
                i = count;
                matchFound = true;
            }
        }

        if (!matchFound) {
            StringBuilder list = new StringBuilder();
            for (final PrintService aService : service) {
                list .append('\"').append(aService.getName()).append("\",");
            }

            reportError("Printer " + newPrinter + " not supported. Options=" + list);
        }

        return matchFound;
    }

    /** single routine to log activity for easy debugging */
    private static void logMessage(final String message) {

        // change to suit your needs

        //System.out.println(message);
        LogWriter.writeLog(message);

    }

    /** single routine so error handling can be easily setup */
    private static void reportError(final String message)  {
        // change to suit your needs
        System.err.println(message);
        LogWriter.writeLog(message);
    }

    /**
     * main routine which checks for any files passed and runs the demo
     */
    public static void main(final String[] args) {

        logMessage("Simple demo to print pages");

        // check user has passed us a filename and use default if none
        if (args.length != 2) {

            System.out.println("Printing needs 2 parameters");
            System.out.println("Parameter 1 - File name or directory (put in quotes if it contains spaces");
            System.out.println("Parameter 2- a printer name");

            System.out.println("---Available printers are---");

            try {
                final PrintService[] service = PrinterJob.lookupPrintServices(); // list of printers

                for (final PrintService aService : service) {
                    System.out.println('"' + aService.getName() + '"');
                }

            }
            catch (final Exception e) {
                e.printStackTrace(); 
            }
            
            System.exit(0);
        }
        else {

            final String file_name = args[0];
            final String printerName = args[1];

            logMessage("File :" + file_name);
            logMessage("Printer :" + printerName);

            // check printer exists
            final boolean validPrinter = validatePrinter(printerName);

            // check file exists
            final File pdf_file = new File(file_name);

            // if file exists, open and get number of pages
            if (!pdf_file.exists()) {
                logMessage("File " + file_name + " not found");

            }
            else if (!validPrinter) {
                logMessage("Printer " + printerName + " not found");

            }
            else {
                new SilentPrint(file_name,printerName);
            }
        }
    }

    /**
     * listener code - just an example
     */
    private static class PDFPrintJobListener implements PrintJobListener {

        private static final boolean showMessages = false;

        @Override
        public void printDataTransferCompleted(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printDataTransferCompleted=" + printJobEvent);
            }
        }

        @Override
        public void printJobCompleted(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printJobCompleted=" + printJobEvent);
            }
        }

        @Override
        public void printJobFailed(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printJobEvent=" + printJobEvent);
            }
        }

        @Override
        public void printJobCanceled(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printJobFailed=" + printJobEvent);
            }
        }

        @Override
        public void printJobNoMoreEvents(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printJobNoMoreEvents=" + printJobEvent);
            }
        }

        @Override
        public void printJobRequiresAttention(final PrintJobEvent printJobEvent) {
            if (showMessages) {
                System.out.println("printJobRequiresAttention=" + printJobEvent);
            }
        }
    }
}