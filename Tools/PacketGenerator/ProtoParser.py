class ProtoParser(object):
    def __init__(self, recv_prefixes, send_prefixes):
        self.recv_pkt = []
        self.send_pkt = []
        self.total_pkt = []
        self.recv_prefixes = recv_prefixes if isinstance(recv_prefixes, list) else [recv_prefixes]
        self.send_prefixes = send_prefixes if isinstance(send_prefixes, list) else [send_prefixes]

        # prefix별 시작 ID 계산 (첫번째 1만번대, 두번째 2만번대 ...)
        all_prefixes = self.recv_prefixes + self.send_prefixes
        unique_first_chars = []
        for p in all_prefixes:
            first_char = p[0]
            if first_char not in unique_first_chars:
                unique_first_chars.append(first_char)

        self.prefix_id_map = {}
        for i, char in enumerate(unique_first_chars):
            self.prefix_id_map[char] = (i + 1) * 10000

    def get_next_id(self, pkt_name):
        first_char = pkt_name[0]
        id = self.prefix_id_map[first_char]
        self.prefix_id_map[first_char] += 1
        return id

    def parse_proto(self, path):
        f = open(path, 'r', encoding='utf-8')
        lines = f.readlines()

        for line in lines:
            if line.startswith('message') == False:
                continue

            pkt_name = line.split()[1].upper()
            if any(pkt_name.startswith(p.upper()) for p in self.recv_prefixes):
                id = self.get_next_id(pkt_name)
                self.recv_pkt.append(Packet(pkt_name, id))
                self.total_pkt.append(Packet(pkt_name, id))
            elif any(pkt_name.startswith(p.upper()) for p in self.send_prefixes):
                id = self.get_next_id(pkt_name)
                self.send_pkt.append(Packet(pkt_name, id))
                self.total_pkt.append(Packet(pkt_name, id))
        f.close()

class Packet:
    def __init__(self, name, id):
        self.name = name
        self.id = id