from TestUtil.SIoTNemeaTester import SIoTNemeaTester
from TestUtil.Shell import Shell
from argparse import ArgumentParser
import json
import subprocess
from shutil import copy2
from pathlib import Path

class IntegrationTest(SIoTNemeaTester):
    def __init__(self, arg: ArgumentParser):
        super.__init__(arg)   
        self.p = False

    # This method backups NEMEA SupervisorL configuration which is already
    # present at the router
    def __t_backup_sv_conf__(self):
        sv_conf = self.__t_meta['config']
        if sv_conf.is_file():
            sv_conf_backup = Path('{}/{}.backup'.format(os.getcwd(), sv_conf.name))
            copy2(sv_conf, sv_conf_backup)
            self.__t_meta['config-backup'] = sv_conf_backup

    # This method serves to restore the original NEMEA SupervisorL configuration
    # from the backup
    def __t_restore_sv_conf__(self):
        if self.__t_meta['config-backup'].is_file():
            self.__t_cp_to_sv_conf_loc__(self.__t_meta['config-backup'])
            os.remove(self.__t_meta['config-backup'])

    # Copies a file to location of the NEMEA SupervisorL configuration file
    def __t_cp_to_sv_conf_loc__(self, path: Path):
        if not path.is_file():
            print('hm')

        copy2(path, self.__t_meta['config'])
        
    def __clean_action__(self):
            self.__t_restore_sv_conf__()
    
    def run(self) -> int:
        self.__prepare_test_output_directories__(self.repo.modules)
        supervisor = Path("/etc/init.d/nemea-supervisorl")
        if not supervisor.is_file():
            raise RuntimeError('NEMEA SupervisorL not found: {} is not a file'.format(supervisor))
            
        self.__t_backup_sv_conf__()
        self.__t_cp_to_sv_conf_loc__(self.__t_meta['test-config'])
        subprocess.Popen([supervisor, 'reload'])
        subprocess.Popen([supervisor, 'start'])
        
        if self.p and not self.quit:
            input("You can now verify whether the modules are running - Press Enter to continue...")
        
        for m in self.repo.modules:
            if not self.repo.got_tests(m):
                continue
            
            tests = self.repo.get_tests(m)
            
            if m == 'ble-pairing':
                x = Shell()
                x.execute('mkdir /tmp/ble-pairing-test/')
            
            for t in tests:
                if self.quit:
                    return 2
                
                if m == 'ble-pairing':
                    x = Shell()
                    x.execute('rm -rf /tmp/ble-pairing-test/*')
                
                self.__test_started__(m, t)
                
                f_in = self.__get_test_in_path__(m, t)
                f_out = self.__get_test_out_path__(m, t)
                f_exp = self.__get_test_expected_out_path__(m, t)
                ifc_in = 'u:test-in'
                ifc_out = 'u:test-out'
                                        
                self.__inject_and_capture__(ifc_in, f_in, ifc_out, f_out)
                self.__test_ended__(self.__compare_files__(f_exp, f_out))
            
            if m == 'ble-pairing':
                x = Shell()
                x.execute('rm -rf /tmp/ble-pairing-test/')
            
        return self.__finished__()
        
        
def main():
    parser = ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-p', action='store_true', help='Pauses the script before and after the test so that it is possible to verify wether modules are correctly running')
    integration_test = IntegrationTest(parser)
    exit(integration_test.run())


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print('\n[!] ERROR: {}'.format(e))
        exit(3)
