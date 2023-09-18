import numpy as np
import os


def insert_line(script: list, line_num: int, line: int) -> None:
    x = -1 if script[-1] == '#Execute script' else 0
    line_num -= 1  # cuz main():
    if len(script) - 2 + x == line_num:
        script.insert(line_num, line)

    elif len(script) - 2 + x > line_num:
        script[line_num] = line
    else:
        # case when line comes not in the same order
        [script.insert(i - 2, '\n')
         for i in range(len(script) + x, line_num + 2)]
        script.insert(line_num, line)


def writing_file(num_line: int, script_txt: str, file_path) -> None:
    # defultive script
    """
    def main():
        ... your script
    if __name__ == "__main__"
        main()
    """
    script_txt = script_txt.replace('\n', '\n\t').strip('\t')
    Tab = '\t'
    line = Tab + script_txt

    num_line += 1  # cuz main():

    if os.path.isfile(file_path):
        with open(file_path, 'r') as f:
            file = f.readlines()
    else:
        file = ['def main():\n', 'if __name__ == "__main__":\n',
                '    main()\n']

    # if not any(arg in line for arg in ['\n', '#Execute script']):
    if line.find('\n') == -1:
        line += '\n'
        if file[-1] != '#Execute script':
            file.append('#Execute script')

    insert_line(file, num_line, line)

    with open(file_path, 'w') as f:
        f.writelines(file)


def reset_file(script_path: str) -> None:
    file = ['def main():\n', 'if __name__ == "__main__":\n',
            '    main()\n']

    with open(script_path, 'w') as f:
        f.writelines(file)


def execute_file(file_name: str, script_path: str) -> list:
    def module_from_file(module_name, file_path):
        import importlib.util

        spec = importlib.util.spec_from_file_location(module_name, file_path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module
    try:
        script = module_from_file(file_name,  script_path)
        result = str(script.main())
        run_success = True
        info = [run_success, file_name, len(result), result]

    except Exception as e:
        import sys
        from traceback import format_tb
        exception_type, exception_object, exception_traceback = sys.exc_info()
        eror = format_tb(exception_traceback)[-1]
        print(eror)
        line_num = int(eror[eror.find(',') + 1: eror.rfind(',')
                            ].replace('line', '').strip())

        if isinstance(exception_object, SyntaxError):  # bypass problem in that eror
            line_num = exception_object.lineno
        run_success = False
        line_num -= 1  # cuz main()
        info = [run_success, file_name, line_num]

    print(info)
    return info


def check_execute(file_name, script_line, script_path):
    if not script_line.endswith("\n"):
        return True
    with open(script_path, 'r') as f:
        last_line = f.readlines()[-1]
        return True if last_line == '#Execute script' else False


def writeMetaFile(mission_count: int, data: list) -> list:
    run_success, file_name = data[:2]
    if run_success:
        chars_length, result = data[2:]
    else:
        eror_line_num = data[2:]

    FINAL_PATH = f"./outbox/{mission_count}/_metaUploadingFile.bin"

    file_num = int(''.join(i for i in file_name if i.isdigit()))

    with open(FINAL_PATH, 'wb+') as f:
        def hex_byte_trans(x): return np.array(x).astype(np.uint8).tobytes()

        b_run_success = hex_byte_trans(run_success)
        b_file_num = hex_byte_trans(file_num)

        f.write(b_run_success)
        f.write(b_file_num)

        if run_success:
            b_chars_length = hex_byte_trans(chars_length)

            f.write(b_chars_length)
            f.write(bytes(result, 'utf-8'))
            print(b_run_success + b_file_num +
                  b_chars_length + bytes(result, 'utf-8'))

        else:
            b_eror_line_num = hex_byte_trans(eror_line_num)
            f.write(b_eror_line_num)
            print(b_run_success + b_file_num + b_eror_line_num)


def main(mission_count: int, script_num: int, line_num: int, txt: str, reset):
    path = "/home/pi/scripts_folder/"
    if isinstance(path, str) and not os.path.exists(path):
        os.makedirs(path)

    file_name = 'script' + str(script_num) + '.py'
    script_path = path + file_name

    if reset or not os.path.exists(script_path):
        reset_file(script_path)

    writing_file(line_num, txt, script_path)

    if check_execute(file_name, txt, script_path):
        import time
        time.sleep(0.5)
        info = execute_file(file_name, script_path)
        writeMetaFile(mission_count, info)
    else:
        FINAL_PATH = f"./outbox/{mission_count}/_metaUploadingFile.bin"
        with open(FINAL_PATH, 'wb+') as f:
            f.write(np.array(script_num).astype(np.uint8).tobytes())
            f.write(np.array(line_num).astype(np.uint8).tobytes())
            print(
                f'changed line: {line_num}, {np.array(line_num).astype(np.uint8).tobytes()}')
