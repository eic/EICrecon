import subprocess
import datetime


class SimConfig:

    def __init__(self):
        self.events_num = 2000
        self.particle = "e-"
        self.e_min = 0.01   # [GeV]
        self.e_max = 30     # [GeV]
        self.multiplicity = 1
        self.detector_config = "epic_arches"
        self.file_date = datetime.datetime.now().strftime('%Y-%m-%d')

    @property
    def output_file_name(self):
        return f'{self.file_date}_pgun_{self.particle}_{self.detector_config}_e{self.e_min}-{self.e_max}GeV_alldir_{self.multiplicity}prt_{self.events_num}evt.edm4hep.root'

    @property
    def sim_command_list(self):
        """Command to run simulation as a list of ddsim+flags"""
        sim_command = [
            f'ddsim',
            f'--compactFile=$DETECTOR_PATH/{self.detector_config}.xml',
            f'-N={self.events_num}',
            f'--random.seed=1',
            f'--enableGun',
            f'--gun.particle="e-"',
            f'--gun.momentumMin={self.e_min}*GeV',
            f'--gun.momentumMax={self.e_max}*GeV',
            # f'--gun.thetaMin=0*deg',
            # f'--gun.thetaMax=90*deg',
            f'--gun.distribution=uniform',
            f'--outputFile={self.output_file_name}',
        ]
        return sim_command

    @property
    def sim_command(self):
        """Command to run simulation in one line"""
        return " ".join(self.sim_command_list)


def upload_to_s3(file_name):
    """Upload file to amazon S3 using AWS CLI"""
    cmd = f"aws s3api put-object --bucket eicaidata --key {file_name} --body {file_name}"

    start_time = datetime.datetime.now()
    print(f"S3 upload of '{file_name}':")
    print(f"S3 upload start time: {start_time.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"S3 Command:\n    {cmd}")

    # run! run! upload!
    subprocess.run(cmd, shell=True, check=True)

    # reporting end
    end_time = datetime.datetime.now()
    print(f"S3 ended at: {end_time.strftime('%Y-%m-%d %H:%M:%S')}, duration {end_time - start_time}: ")


def run_simulation(config: SimConfig, upload=False):
    """Runs simulation based on SimConfig and upload resulting data to S3 (if you have an access and flag is set)"""

    start_time = datetime.datetime.now()
    print(f"Simulation start time: {start_time.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Simulation Command:\n    {config.sim_command}")

    # run! run! upload!
    subprocess.run(config.sim_command, shell=True, check=True)

    # reporting end
    end_time = datetime.datetime.now()
    print(f"Simulation ended at: {end_time.strftime('%Y-%m-%d %H:%M:%S')}, duration {end_time - start_time}: ")

    if upload:
        upload_to_s3(config.output_file_name)


if __name__ == "__main__":
    # 1 electron
    cfg = SimConfig()
    cfg.multiplicity = 1
    cfg.particle = "pi-"
    # cfg.events_num = 10
    # run_simulation(cfg, should_upload)
    #
    # cfg.events_num = 100
    # run_simulation(cfg, should_upload)
    cfg.detector_config = "epic_arches"

    cfg.events_num = 1000
    run_simulation(cfg)
