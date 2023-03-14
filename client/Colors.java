public enum Colors {
    BLACK,
    BLUE,
    BROWN,
    GREEN,
    ORANGE,
    PINK,
    RED,
    YELLOW,
    MAX_COUNT;

    String Colors(int value) { return Colors.values()[value].name(); }
}
