import reco_flags_eicrecon

flag_records = reco_flags.eicrecon_reco_flags

csv_content = "# flag_name, default_val, description\n"

for record in flag_records:
    flag_name = "'{}',".format(record[0])
    csv_content += f'"{record[0]}", "{record[1]}", "{record[2]}"\n'
    print(record)


with open("reco_flags_dump.csv", "w") as text_file:
    text_file.write(csv_content)

