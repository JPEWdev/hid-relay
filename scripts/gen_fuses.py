#! /usr/bin/env python3
#
# Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0

import argparse
import sys
import contextlib


@contextlib.contextmanager
def output_file(path, mode):
    if path == "-":
        yield sys.stdout
    else:
        with open(path, mode) as f:
            yield f


def main():
    parser = argparse.ArgumentParser(
        description="Write fuse config for programming with avrdude"
    )
    parser.add_argument("--cpu", help="CPU", required=True)
    parser.add_argument("--lfuse", help="Low Fuses", required=True)
    parser.add_argument("--hfuse", help="High Fuses", required=True)
    parser.add_argument("--efuse", help="Extended Fuses", default="")
    parser.add_argument(
        "--output",
        help="Output file, or '-' for stdout (Default is %(default)s",
        default="-",
    )
    args = parser.parse_args()

    with output_file(args.output, "w") as f:
        f.write(f"-p {args.cpu} ")
        f.write("-B 100 ")
        f.write(f"-U lfuse:w:{args.lfuse}:m ")
        f.write(f"-U hfuse:w:{args.hfuse}:m ")
        if args.efuse:
            f.write(f"-U efuse:w:{args.efuse}:m ")
        f.write("\n")


if __name__ == "__main__":
    sys.exit(main())
