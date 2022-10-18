import io
import reco_flags_eicrecon
from system_of_units import eV, MeV, GeV, mm, cm, mrad

flag_records = reco_flags.eicrecon_reco_flags

# For some values we need to eval the result
known_units_list = ['eV', 'MeV', 'GeV', 'mm', 'cm', 'mrad']

print(MeV)

def has_unit_conversion(value):
    for unit_name in known_units_list:
        if f'*{unit_name}' in value or \
           f'* {unit_name}' in value or \
           f'/{unit_name}' in value or \
           f'/ {unit_name}' in value:
            return True
    return False


def value_eval(value):

    if has_unit_conversion(value):
        # need evaluation of unit conversion
        return str(eval(value))

    if 'capacityBitsADC' in value:
        # Value given in a form of 'capacityBitsADC=8'
        capacity_bits = 2 ** int(value.split("=")[1])
        return str(capacity_bits)

    return value


with io.open("launch_script.in.py") as input_file:
    text = input_file.read()

    flags_text = "reco_parameters_flags = [\n"

    for record in flag_records:
        if record[1]:
            value = value_eval(record[1])
            comment = f"     # {record[2]}" if record[2] else ""
            flags_text += f'"-P{record[0]}={value}",{comment}\n'

    flags_text +="]\n"
    print(flags_text)

    text = text.replace("reco_parameters_flags = []", flags_text)

    with open("launch_script.py", "w") as output_file:
        output_file.write(text)

