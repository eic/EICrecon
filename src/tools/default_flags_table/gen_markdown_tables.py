# Usage
# python3 gen_markdown_tables.py
#   --all-flags=flags.json
#   --reco-flags=reco_flags.json
#   --good-reco-flags=good_reco_flags.json
#   --template=../../docs/tables/table.in.md
#   --output=tables.md
# For CLion debug example from default_flags_table
# gen_markdown_tables.py
#    --flags=$ContentRoot$/src/tools/default_flags_table/all_flags.json
#    --good-flags=$ContentRoot$/src/tools/default_flags_table/all_flags.json
#    --template=$ContentRoot$/docs/table_flags/flags.in.md
#    --output=$ContentRoot$/docs/table_flags/flags.md

import json
import argparse
import re



reco_prefixes = [
    "B0TRK",
    "BEMC",
    "BTRK",
    "BVTX",
    "ECGEM",
    "ECTRK",
    "EEMC",
    "FOFFMTRK",
    "HCAL",
    "MPGD",
    "RPOTS",
    "ZDC",
    "Tracking",
    "Reco",
    "Digi",
    "Calorimetry"
]

# Case independent representation of prefixes to compare
low_case_reco_prefixes = [r.casefold() for r in reco_prefixes]


def filter_only_reco_flags(all_records):
    # Here we will save only flag records corresponding to reconstruction
    reco_flags = []

    # Go over reco_prefixes
    for reco_prefix in reco_prefixes:
        print(reco_prefix)

        # Find all flags that starts with this prefix (excluding loggings and tags)
        reco_flag_records = [r
                             for r in all_records
                             if r[0].casefold().startswith(reco_prefix.lower())
                             and 'LogLevel' not in r[0]
                             and 'InputTags' not in r[0]
                             and 'input_tags' not in r[0]
                             and 'verbose' not in r[0]]

        reco_flags.extend(reco_flag_records)

    return reco_flags


def gen_html_table(records):
    table_html = ""
    for record in records:
        print(f"{record[0]}: '{record[1]}' |  '{record[2]}'")
        value = record[1] if len(record[1]) <= 50 else record[1][:70] + "... (!) value is too long"
        table_html += f"<tr><td>{record[0]}</td><td>{value}</td><td>{record[2]}</td></tr>"
    return table_html


def gen_diff_table_html(dump_reco_flags, user_reco_flags):
    print("---------------------------\n Difference\n---------------------------")
    dump_flag_names = [value[0] for value in dump_reco_flags]
    user_flag_names = [value[0] for value in user_reco_flags]
    dump_names_only = [name for name in dump_flag_names if name not in user_flag_names]
    user_names_only = [name for name in user_flag_names if name not in dump_flag_names]

    dump_name_values = {}
    for r in dump_reco_flags:
        dump_name_values[r[0]] = r[1]

    comparison_table = ""

    for user_reco_flag in user_reco_flags:
        to_compare_user_flag_name = user_reco_flag[0]
        to_compare_user_flag_value = user_reco_flag[1]
        if to_compare_user_flag_name in dump_name_values:
            to_compare_dump_flag_value = dump_name_values[to_compare_user_flag_name]
            if to_compare_dump_flag_value != to_compare_user_flag_value:
                print(f"{to_compare_user_flag_name}: '{to_compare_user_flag_value}' |  '{to_compare_dump_flag_value}'")
                comparison_table += f"<tr><td>{to_compare_user_flag_name}</td><td>{to_compare_user_flag_value}</td><td>{to_compare_dump_flag_value}</td></tr>"

    return comparison_table


def gen_mardown_page(template_file_name, out_file_name, reco_flags, good_reco_flags, all_flags):

    with open(template_file_name, "r", encoding='utf-8') as in_file:
        input_data = in_file.read()

    reco_flags_table = gen_html_table(reco_flags)
    good_reco_flags_table = gen_html_table(good_reco_flags)
    all_flags_table = gen_html_table(all_flags)
    comparison_table = gen_diff_table_html(reco_flags, good_reco_flags)

    templates_to_datasources = {
        "TABLE1": reco_flags_table,
        "TABLE2": good_reco_flags_table,
        "TABLE3": comparison_table,
        "TABLE4": all_flags_table,
    }

    def build_re_for_key(key):
        return f"<!--{key} BEGIN-->.*<!--{key} END-->"

    result = input_data
    for template_name, data in templates_to_datasources.items():
        result = re.sub(build_re_for_key(template_name), data, result, flags=(re.DOTALL | re.MULTILINE))

    with open(out_file_name, "w", encoding='utf-8') as out_file:
        out_file.write(result)


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('--flags', help="JSon file with all flags")
    parser.add_argument('--good-flags',  help="JSon file with good default flags")
    parser.add_argument('--template',  help="Markdown template file name")
    parser.add_argument('--output',  help="Output file name")
    args = parser.parse_args()

    # Open all flags
    all_flags = []
    with open(args.flags) as f:
        all_flags = json.load(f)

    # Open file with all good flags
    good_all_flags = []
    with open(args.good_flags) as f:
        good_all_flags = json.load(f)

    # Filter to have only reco flags
    reco_flags = filter_only_reco_flags(all_flags)
    good_reco_flags = filter_only_reco_flags(good_all_flags)

    # generate markdown page
    gen_mardown_page(args.template, args.output, reco_flags, good_reco_flags, all_flags)

