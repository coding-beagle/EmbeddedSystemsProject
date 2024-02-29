timer = 0.0

def go_straight(duration, speed):
    global timer
    time_end = timer + duration
    output = f"    {{{timer:.2f}, {time_end:.3f}, 1,0, {speed}}},\n"
    timer = time_end
    return output

def turn_left(duration, speed):
    global timer
    time_end = timer + duration
    output = f"    {{{timer:.2f}, {time_end:.3f}, 1,1, {speed}}},\n"
    timer = time_end
    return output

def turn_right(duration, speed):
    global timer
    time_end = timer + duration
    output = f"    {{{timer:.2f}, {time_end:.3f}, 0,0, {speed}}},\n"
    timer = time_end
    return output

def delay(duration):
    global timer
    time_end = timer + duration
    output = f"    {{{timer:.2f}, {time_end:.3f}, 0,0, 0.0}},\n"
    timer = time_end
    return output

output = ""

output += delay(0.01)

output += go_straight(2.4, 3.0)
output += delay(0.2)
output += turn_left(0.53, 3.0)
output += delay(0.2)

for i in range(3):
    output += go_straight(2, 3.0)
    output += delay(0.2)
    output += turn_left(0.53, 3.0)
    output += delay(0.2)

output += turn_left(0.53, 3.0)
output += delay(0.2)

for i in range(4):
    output += go_straight(2, 3.0)
    output += delay(0.2)
    output += turn_right(0.4, 3.0)
    output += delay(0.2)

output += f"    {{{timer:.2f}, -1, 1, 0, 0.0}}\n"

print(output)

