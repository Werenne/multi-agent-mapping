""" 
    Description.
"""


import serial
import time
from threading import Thread
from abc import ABCMeta, abstractmethod
from queue import Queue

from utils import Container


#---------------
class Messenger(Thread, metaclass=ABCMeta):
    """
    For the moment do as if only one robot. Blabla.
    """

    def __init__(self, params, queues):
        Thread.__init__(self)
        self.params = params
        self.q = queues
        self.build_communication()
        

    #---------------
    @abstractmethod
    def build_communication(self):
        pass


    #---------------
    def run(self):
        t1 = Thread(target=self.check_msg_from_master).start()
        t2 = Thread(target=self.check_msg_from_robots).start()


    #---------------
    def check_msg_from_master(self):
        while True:
            directive = self.q.master2messenger.get()
            if directive.type_directive == "instruction":
                msg = self.make_msg(directive.id_robot,
                        directive.instruction)
                self.send_msg_to_robot(directive.id_robot, msg)


    #---------------
    def check_msg_from_robots(self):
        while True:
            for id_robot in self.robots.keys():
                valid_msg = self.check_msg_from_robot(id_robot)
                if valid_msg != None:
                    self.send_information_to_master(valid_msg)
            time.sleep(0.5)


    #---------------
    @abstractmethod
    def check_msg_from_robot(self, id_robot=None):
        pass
       

    #---------------
    @abstractmethod
    def send_msg_to_robot(self, id_robot, msg):
        pass
  

    #---------------
    def send_information_to_master(self, msg):
        directive = {
            "type_directive": "information",
            "id_robot": msg.id_robot,
            "information": {
                "type_intersection": msg.type_intersection, 
                "distance": msg.distance 
            }
        }
        self.q.messenger2master.put(Container(directive))


    #---------------
    def process_msg_from_robot(self, id_robot, raw_msg):
        if not self.valid_syntax(raw_msg):
            return None
        parsed_msg = self.parse_msg(raw_msg)
        print(parsed_msg)
        return self.valid_msg(parsed_msg)


    #---------------
    def parse_msg(self, msg):
        msg = msg[1:-1]  # remove delimeters
        (id_robot, seq_number, information) = msg.split('/') 
        (type_intersection, distance) = information.split(';')
        parsed_msg = {}
        parsed_msg['id_robot'] = int(id_robot) 
        parsed_msg['seq_number'] = int(seq_number)
        parsed_msg['type_intersection'] = int(type_intersection)
        parsed_msg['distance'] = self.discretize(float(distance))
        return Container(parsed_msg)


    #---------------
    def make_msg(self, id_robot, instruction):
        seq_number = self.robots[id_robot]["seq_number_sending"]
        msg = "<" + str(id_robot) + "/" + str(seq_number) + \
                    "/" + str(instruction) + ">"
        return msg
                

    #---------------
    def valid_msg(self, parsed_msg):
        if not self.valid_checksum(parsed_msg):
            return None
        if not self.valid_order(parsed_msg):
            return None
        if not self.valid_args(parsed_msg):
            return None
        id_robot = parsed_msg.id_robot
        self.robots[id_robot]["seq_number_receiving"] = parsed_msg.seq_number
        return parsed_msg


    #---------------
    def valid_args(self, msg):
        return msg.distance > 0


    #---------------
    def valid_syntax(self, msg):
        return len(msg) > 0 and msg[0] == '<' and msg[-1] == '>' 


    #---------------
    def valid_checksum(self, msg):
        return True


    #---------------
    def valid_order(self, msg):
        return msg.seq_number >= self.robots[msg.id_robot]["seq_number_receiving"]


    # ------------
    def discretize(self, distance):
        return int(distance/5.) * 5



#---------------
class MessengerReal(Messenger):
    """    
    Description.
    """

    def __init__(self, params, queues):
        super().__init__(params, queues)


    #---------------
    def build_communication(self):
        self.id_master = 0
        self.n_robots = self.params.n_robots
        self.robots = {}
        for robot_id, robot_comm in self.params.robots.items():
            temp = {}
            temp["serial"] = serial.Serial(robot_comm["port"],
                    robot_comm["baud_rate"])
            temp["seq_number_sending"] = 0
            temp["seq_number_receiving"] = 0
            self.robots[robot_id] = temp
        self.robots = Container(self.robots)


    #---------------
    def send_msg_to_robot(self, id_robot, msg):
        self.robots[id_robot]["serial"].write(msg.encode())
        self.robots[id_robot]["seq_number_sending"] += 1


    #---------------
    def check_msg_from_robot(self, id_robot=None):
        serial = self.robots[id_robot]["serial"]    
        raw_msg = serial.readline().decode("utf-8").rstrip();
        print(raw_msg)
        return self.process_msg_from_robot(id_robot, raw_msg)



#---------------
class MessengerSimul(Messenger):
    """    
    Description.
    """

    def __init__(self, params, queues):
        super().__init__(params, queues)


    #---------------
    def build_communication(self):
        self.id_master = 0
        self.n_robots = self.params.n_robots
        self.robots = {}
        for i in range(self.n_robots):
            id_robot = i+1
            comm = {}
            comm["seq_number_sending"] = 0
            comm["seq_number_receiving"] = 0
            self.robots[id_robot] = comm
        self.robots = Container(self.robots)


    #---------------
    def send_msg_to_robot(self, id_robot, msg):
        self.q.messenger2robots.put(msg)
        self.robots[id_robot]["seq_number_sending"] += 1


    #---------------
    def check_msg_from_robot(self, id_robot=None):
        raw_msg = self.q.robots2messenger.get()  
        return self.process_msg_from_robot(id_robot, raw_msg)

















        