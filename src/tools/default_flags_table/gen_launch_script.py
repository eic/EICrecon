import io
import reco_flags

flag_records = reco_flags.eicrecon_reco_flags

with io.open("launch_script.in.py") as input_file:
    text = input_file.read()


    flags_text = "reco_parameters_flags = [\n"

    for record in flag_records:
        if record[1]:
            comment = f"     # {record[2]}" if record[2] else ""
            flags_text += f'"-P{record[0]}={record[1]}{comment}",\n'


    flags_text +="]\n"
    print(flags_text)

    text = text.replace("reco_parameters_flags = []", flags_text)

    with open("launch_script.py", "w") as output_file:
        output_file.write(flags_text)

