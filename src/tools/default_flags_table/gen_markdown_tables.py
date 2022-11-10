# Usage
# python3 gen_markdown_tables.py
#    --flags=$ContentRoot$/src/tools/default_flags_table/arches_flags.json
#    --template=$ContentRoot$/docs/table_flags/flags.in.md
#    --output=$ContentRoot$/docs/table_flags/flags.md

import json
import argparse
import re


reco_prefixes = [
    "B0ECAL",
    "B0TRK",
    "BEMC",
    "BTOF",
    "BTRK",
    "BVTX",
    "ECGEM",
    "ECTOF",
    "ECTRK",
    "EEMC",
    "FOFFMTRK",
    "HCAL",
    "MPGD",
    "RICH",
    "RPOTS",
    "ZDC",
    "Tracking",
    "Reco",
    "Digi",
    "Calorimetry"
]

# Case independent representation of prefixes to compare
casefolded_prefixes = [r.casefold() for r in reco_prefixes]


def filter_only_reco_flags(records):
    # Here we will save only flag records corresponding to reconstruction

    for record in records:
        # by default each record has 4 values: flag_name, value, default-value, description
        # we add 5th value - is it reconstruction related flag or not
        is_reco = record[0].casefold().startswith(tuple(casefolded_prefixes))  \
                  and 'LogLevel' not in record[0] \
                  and 'InputTags' not in record[0] \
                  and 'input_tags' not in record[0] \
                  and 'verbose' not in record[0]
        record.append(is_reco)
    return records


def gen_html_table(records):
    values_table_html = ""

    for record in records:
        # value = record[1] if len(record[1]) <= 50 else record[1][:70] + "... (!) value is too long"
        # def_value = record[2] if len(record[2]) <= 50 else record[2][:70] + "... (!) value is too long"
        value = record[1] if len(record[1]) <= 50 else "... (!) value is too long"
        def_value = record[2] if len(record[2]) <= 50 else "... (!) value is too long"
        values_table_html += f"<tr><td>{record[0]}</td><td>{value}</td><td>{def_value}</td><td>{record[4]}</td><td>{record[3]}</td></tr>"
    return values_table_html



def gen_mardown_page(template_file_name, out_file_name, flags):

    with open(template_file_name, "r", encoding='utf-8') as in_file:
        input_data = in_file.read()

    html_table = gen_html_table(flags)

    templates_to_datasources = {
        "TABLE": html_table
    }

    result = re.sub("<!--TABLE BEGIN-->.*<!--TABLE END-->", html_table, input_data, flags=(re.DOTALL | re.MULTILINE))

    with open(out_file_name, "w", encoding='utf-8') as out_file:
        out_file.write(result)


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('--flags', help="JSon file with all flags")
    parser.add_argument('--template',  help="Markdown template file name")
    parser.add_argument('--output',  help="Output file name")
    args = parser.parse_args()

    # Open all flags
    flags = []
    with open(args.flags) as f:
        flags = json.load(f)

    # Filter to have only reco flags
    reco_flags = filter_only_reco_flags(flags)

    # generate markdown page
    gen_mardown_page(args.template, args.output, flags)

