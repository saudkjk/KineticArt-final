
import cv2
import mediapipe as mp
import numpy as np
import serial.tools.list_ports
import serial
import time

def setup_serial():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(port)
    selected_port = input('Select Port (e.g., COM3): ')
    serial_inst = serial.Serial()
    serial_inst.baudrate = 9600
    serial_inst.port = selected_port
    serial_inst.open()
    return serial_inst

def send_row_state(serial_inst, row, states):
    state_string = ''.join(str(int(state)) for state in states)
    command = f"{state_string},{row}\n"
    serial_inst.write(command.encode('utf-8'))

mp_selfie_segmentation = mp.solutions.selfie_segmentation
segmentation_model = mp_selfie_segmentation.SelfieSegmentation(model_selection=1)

cap = cv2.VideoCapture(0)
serial_inst = setup_serial()

previous_state = np.zeros((9, 30), dtype=bool)  
last_update_time = np.zeros(9) 
update_interval = 5  
slowed = False  

try:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            continue

        image = cv2.cvtColor(cv2.flip(image, 1), cv2.COLOR_BGR2RGB)
        results = segmentation_model.process(image)
        body_mask = results.segmentation_mask > 0.1
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

        image_height, image_width, _ = image.shape
        grid_rows, grid_cols = 9, 30
        original_radius = max(image_width // (grid_cols * 3), 2)
        circle_radius = 2 * original_radius

        vertical_spacing = image_height // grid_rows
        horizontal_spacing = image_width // grid_cols

        current_time = time.time()
        for row in range(grid_rows):
            row_states = []
            for col in range(grid_cols):
                x = int(col * horizontal_spacing + horizontal_spacing / 2)
                y = int(row * vertical_spacing + vertical_spacing / 2)
                present = body_mask[y, x]
                color = (0, 255, 0) if present else (255, 0, 0)
                cv2.circle(image, (x, y), circle_radius, color, -1)
                row_states.append(present)

            if slowed:
                if not np.array_equal(previous_state[row], row_states) and (current_time - last_update_time[row] >= update_interval):
                    send_row_state(serial_inst, row, row_states)
                    last_update_time[row] = current_time
                    previous_state[row] = row_states.copy()
            else:
                if not np.array_equal(previous_state[row], row_states):
                    send_row_state(serial_inst, row, row_states)
                    previous_state[row] = row_states.copy()

        cv2.imshow('MediaPipe Selfie Segmentation with Grid Overlay', image)
        if cv2.waitKey(5) & 0xFF == 27:  # ESC key
            break
finally:
    cap.release()
    cv2.destroyAllWindows()
    serial_inst.close()
