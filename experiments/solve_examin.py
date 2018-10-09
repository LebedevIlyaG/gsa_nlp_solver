import sys, os
import re
import argparse
from subprocess import Popen, PIPE
from benchmark_tools.stats import save_stats, compute_stats
import platform

def create_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('--bin_path', type=str, default='')

    parser.add_argument('--problems_class', type=str, choices=['grish','gklss','gklsh'], default='grish')
    parser.add_argument('--problems_dim', type=int, default=2)
    parser.add_argument('--stop_cond', type=str, default='OptimumVicinity')
    parser.add_argument('--serg_eps', action='store_true')

    parser.add_argument('--max_iters', type=int, default=10000, help='limit of iterations for the method')
    parser.add_argument('--eps', type=float, default=0.01)
    parser.add_argument('--r', type=float, default='2.3')
    parser.add_argument('--lm', type=int, default='0')
    parser.add_argument('--stats_fname', type=str, default='test.json')
    parser.add_argument('--verbose', action='store_true', help='Print additional info to console')

    return parser

def create_parameters_dict(cl_args):
    parameters = {}
    parameters['MaxNP'] = cl_args.max_iters
    parameters['lib'] = os.path.join(os.path.abspath(cl_args.bin_path), \
        get_platform_lib_name(get_lib_name_by_functions_class(cl_args.problems_class)))
    parameters['function_number'] = 1
    parameters['N'] = cl_args.problems_dim
    parameters['stopCond'] = cl_args.stop_cond
    parameters['r'] = cl_args.r
    parameters['lm'] = cl_args.lm
    parameters['Eps'] = cl_args.eps

    if cl_args.problems_class == 'gklss':
        parameters['PC'] = 'Simple'
    elif cl_args.problems_class == 'gklsh':
        parameters['PC'] = 'Hard'

    return parameters

def create_start_command(bin_path, parameters_dict):
    string_start_examin = os.path.join(os.path.abspath(bin_path), get_platform_executable_name('examin'))

    for param in parameters_dict:
        string_start_examin += ' -' + param + ' ' + str(parameters_dict[param])

    return string_start_examin

def start_examin(bin_path, parameters_dict):
    string_start_examin = create_start_command(bin_path, parameters_dict)
    proc = Popen(string_start_examin, shell=True, stdout=PIPE, stderr=PIPE)
    proc.wait()
    output_examin = proc.communicate()

    return output_examin[0]

def get_lib_name_by_functions_class(class_name):
    if 'grish' in class_name:
        return 'grishagin'
    elif 'gkls' in class_name:
        return 'gkls'
    else:
        raise Exception('Invalid functions class name')

def get_platform_lib_name(lib):
    if 'Linux' in platform.system():
        return 'lib' + lib + '.so'
    elif 'Windows' in platform.system():
        return lib + '.dll'
    return None

def get_platform_executable_name(name):
    if 'Linux' in platform.system():
        return name
    elif 'Windows' in platform.system():
        return name + '.exe'
    return None

def start_serial(args):
    information_about_experiments = []
    problem_status = []
    parameters = create_parameters_dict(args)

    for i in range(1,101):
        output_examin = ''
        try:
            parameters['function_number'] = i
            output_examin = start_examin(args.bin_path, parameters)

            number_of_trials = re.search(r'NumberOfTrials = (\d+)', str(output_examin)).group(1)
            information_about_experiments.append([int(number_of_trials)])
            result = re.search(r'FOUND!', str(output_examin))
            if result == None:
                problem_status.append(False)
            else:
                problem_status.append(True)

            if args.verbose:
                print('Problem # {}: '.format(i) + ('solved' if problem_status[-1] else 'not solved') +
                    ' after {} trials'.format(number_of_trials))
        except BaseException as e:
            print(e)
            print('Examin failed:')
            print(output_examin)
            sys.exit()
    stats = compute_stats(information_about_experiments, problem_status)
    print('Problems solved: {}'.format(stats['num_solved']))
    for i, avg in enumerate(stats['avg_calcs'][:-1]):
        print('Average number of calculations of constraint #{}: {}'.format(i, avg))
    print('Average number of calculations of objective: {}'.format(stats['avg_calcs'][-1]))

    return stats

def main(args):
    stats = start_serial(args)
    save_stats(stats, args.stats_fname)

if __name__ == '__main__':
    parser = create_parser()

    main(parser.parse_args())
