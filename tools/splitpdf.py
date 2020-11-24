import argparse
import os
import re
import shutil
import sys


class ImageRecombiner:
    """
    Class to handle splitting a pdf into pages, and recombining
    the pages into separate images.
    """

    def __init__(self, input_file, output_dir, tmp_dir, align_and_merge):
        self.input_file = input_file
        self.output_dir = output_dir
        self.tmp_dir = tmp_dir
        self.root = self.get_root_name(input_file)
        self.align_and_merge = align_and_merge

    def get_root_name(self, input_file_name):
        """Removes the path and file extension."""
        return os.path.splitext(os.path.basename(input_file_name))[0]

    def prepare_dirs(self):
        # clear the temp directory (if any) and recreate.
        if os.path.isdir(self.tmp_dir):
            shutil.rmtree(self.tmp_dir)
        os.makedirs(self.tmp_dir)

        # create output directory
        if not os.path.isdir(self.output_dir):
            os.makedirs(self.output_dir)

    def extract_raw_images(self):
        output_root = os.path.join(self.tmp_dir, self.root)
        command = f"pdfimages -j {self.input_file} {output_root}"
        print(command)
        os.system(command)

    def check_extraction(self):
        scans = os.listdir(self.tmp_dir)
        if not scans:
            raise Exception("no images extracted")
        if len(scans) % 2 == 1:
            raise Exception("odd number of scans")

    def get_input_page(self, scan_number):
        return os.path.join(self.tmp_dir, f"{self.root}-{scan_number}.jpg")

    def get_cut_pattern(self, scan_number):
        return os.path.join(self.tmp_dir, f"{self.root}-{scan_number}-split-%02d.png")

    def get_cut_page(self, scan_number, page_number):
        return os.path.join(
            self.tmp_dir, f"{self.root}-{scan_number}-split-0{page_number}.png"
        )

    def get_merged_page(self, scan_number):
        return os.path.join(self.tmp_dir, f"{self.root}-{scan_number}-merged.png")

    def get_output_page(self, scan_number):
        return os.path.join(self.output_dir, f"{self.root}-{scan_number}.jpg")

    def get_last_number(self, file_name):
        """Gets the last number from a filename, e.g., foo123-001.png -> 001""" 
        matches = re.findall(r"\d+", file_name)
        result = list(matches)
        return result[-1]

    def recombine_images(self):
        scans = os.listdir(tmp_dir)
        scan_numbers = [self.get_last_number(s) for s in scans]
        scan_numbers.sort()

        # split into top/bottom halves
        clockwise = True
        for number in scan_numbers:
            input_file = self.get_input_page(number)
            angle = '"90"' if clockwise else '"-90"'
            rotate_command = f"mogrify {input_file} -rotate {angle} {input_file}"
            print(rotate_command)
            os.system(rotate_command)
            output_file_pattern = self.get_cut_pattern(number)
            cut_command = f"convert {input_file} -crop 2x1@ +repage {output_file_pattern}"
            print(cut_command)
            os.system(cut_command)
            clockwise = not clockwise

        # go through the files in pairs, combining pages
        paired_scan_numbers = [
            scan_numbers[i:i + 2] for i in range(0, len(scan_numbers), 2)
        ]
        for pair in paired_scan_numbers:
            page1a = self.get_cut_page(pair[0], 0)
            page1b = self.get_cut_page(pair[1], 0)
            page2a = self.get_cut_page(pair[0], 1)
            page2b = self.get_cut_page(pair[1], 1)
            output1 = self.get_merged_page(pair[0])
            output2 = self.get_merged_page(pair[1])

            command1 = f"{self.align_and_merge} --grayscale {page1a} {page1b} {output1}"
            command2 = f"{self.align_and_merge} --grayscale {page2a} {page2b} {output2}"
            print(command1)
            os.system(command1)
            print(command2)
            os.system(command2)
        
        # color adjustment
        for number in scan_numbers:
            input_file = self.get_merged_page(number)
            output_file = self.get_output_page(number)
            command = f"convert {input_file} -level 14%,86% {output_file}"
            print(command)
            os.system(command)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
    This tool takes a pdf and slices and dices the images to improve the
    image quality.
    
    The pdf should be scans of a book, showing 2 pages at once.
    Each pair of pages in the book should have *two* scans, one
    turned upside down, so the scans might have pages like this:
    [1|2] [2|1] [3|4] [4|3] [5|6] [6|5] ...

    The following steps are done:
    1. extract all the images from the pdf
    2. split each image into two separate page images
    3. combine each page with its upside down image to create
       a higher quality image (basically, this removes any texture 
       from the paper).
    4. each higher-quality page is stored separately in the output 
       directory.
    5. do simple level adjustment to improve contrast
    """)
    parser.add_argument("pdf_scan", help="path to the pdf file to process")
    parser.add_argument("output_dir", help="path of output directory")
    parser.add_argument("align_and_merge", help="path of align_and_merge binary")

    args = parser.parse_args()
    
    tmp_dir = "/tmp/split_pdf_tmp"

    if not os.path.isfile(args.pdf_scan):
        print(f"the pdf file {args.pdf_scan} doesn't exist")
        sys.exit()

    #input_scan_file = "/home/jonathan/Documents/astronomy/astrolog_test/scans/test.pdf"
    #output_dir = "/home/jonathan/Documents/astronomy/astrolog_test/cleaned_scans"
    #align_and_merge_binary = "align_and_merge"

    recombiner = ImageRecombiner(
        args.pdf_scan, args.output_dir, tmp_dir, args.align_and_merge
    )

    recombiner.prepare_dirs()
    recombiner.extract_raw_images()
    recombiner.check_extraction()
    recombiner.recombine_images()
