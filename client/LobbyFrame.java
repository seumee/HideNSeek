import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import java.awt.*;
import java.awt.event.*;

public class LobbyFrame extends JFrame{
    private JPanel lobbyPanel = new JPanel();
    private JTable roomTable;
    private JButton createRoomBtn = new JButton("Create Room");
    private JButton refreshBtn = new JButton("Refresh");
    private JButton[] pageBtn = new JButton[10];


    SocketHandler handler;
    Values values;

    public LobbyFrame(SocketHandler handler, Values values) {
        super("HideNSeek");

        createRoomDialog dialog = new createRoomDialog(this, "Create Room");

        this.handler = handler;
        this.values = values;

        setSize(620, 560);

        String colNames[] = {"RoomNumber", "RoomName", "State"};
        DefaultTableModel model = new DefaultTableModel(colNames, 0) {
            @Override
            public boolean isCellEditable(int row, int column) {
                return false;
            }
        };

        //add Table content
        for(int i = 0; i < values.readValues.roomlist.roomId.length; i++) {
            if(values.readValues.roomlist.roomId[i] == 0) model.addRow(new Object[]{null, null, null});
            //else model.addRow(new Object[]{values.readValues.roomlist.roomId[i], " "+values.readValues.roomlist.roomName[i], values.readValues.roomlist.userCount[i]+"/8 "});
            else model.addRow(new Object[]{values.readValues.roomlist.roomId[i], values.readValues.roomlist.roomName[i], values.readValues.roomlist.userCount[i]+"/8"});
        }

        setTable(handler, values, model);

        ActionListener createRoomListener = e -> dialog.setVisible(true);

        ActionListener pageListener = e -> {
            System.out.println(e.getActionCommand());
            if(!e.getActionCommand().equals("Refresh")) values.readValues.page = Integer.parseInt(e.getActionCommand());
            values.writeValues.page = values.readValues.page;
            values.readValues.msgType = MsgType.GET_PAGE.ordinal();
            new Waiting(handler, values);
            setVisible(false);
        };


        refreshBtn.addActionListener(pageListener);
        lobbyPanel.add(refreshBtn);
        createRoomBtn.addActionListener(createRoomListener);
        lobbyPanel.add(createRoomBtn);
        lobbyPanel.add(roomTable);
        for (int i = 0; i < 10; i++) {
            pageBtn[i] = new JButton(String.valueOf(i + 1));
            pageBtn[i].setBorderPainted(false);
            pageBtn[i].setContentAreaFilled(false);
            pageBtn[i].setFocusPainted(false);
            pageBtn[i].addActionListener(pageListener);
            lobbyPanel.add(pageBtn[i]);
        }

        this.setContentPane(lobbyPanel);
        this.setLocationRelativeTo(null);
        this.setVisible(true);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    }

    private void setTable(SocketHandler handler, Values values, DefaultTableModel model) {
        //set Table Layout
        DefaultTableCellRenderer celAlignCenter = new DefaultTableCellRenderer();
        celAlignCenter.setHorizontalAlignment(JLabel.CENTER);

        roomTable = new JTable(model);
        roomTable.getColumn("RoomNumber").setPreferredWidth(50);
        roomTable.getColumn("RoomNumber").setCellRenderer(celAlignCenter);
        roomTable.getColumn("RoomName").setPreferredWidth(500);
        roomTable.setIntercellSpacing(new Dimension(10, 0));
        roomTable.getColumn("State").setPreferredWidth(50);
        roomTable.getColumn("State").setCellRenderer(celAlignCenter);
        roomTable.addMouseListener(new JTableMouseListener(handler, values.writeValues, values.readValues));
        roomTable.setRowHeight(45);
    }

    class createRoomDialog extends JDialog {
        JTextField roomName = new JTextField(10);
        JButton createBtn = new JButton("Create");

        public createRoomDialog(JFrame frame, String title) {
            super(frame, title);
            this.setLocationRelativeTo(null);
            setLayout(new FlowLayout());
            add(roomName);
            add(createBtn);
            setSize(200, 100);

            Action crtRoom = new AbstractAction() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    values.writeValues.roomname = roomName.getText();
                    values.staticValues.roomname = roomName.getText();
                    values.writeValues.msgType = MsgType.MSG_CRT_ROOM;
                    values.writeValues.writeListener.actionPerformed(null);
                    while(values.readValues.msgType == MsgType.MSG_CRT_ROOM.ordinal()) {}
                    values.staticValues.colorIdx = Colors.BLACK.ordinal();
                    values.staticValues.adminColorIdx = Colors.BLACK.ordinal();
                    values.setDefaultOptions();
                    setVisible(false);
                    frame.setVisible(false);
                    new WaitingRoomFrame(handler, values);
                }
            };

            KeyStroke enter = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0, false);
            roomName.getInputMap(JTable.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(enter, "ENTER");
            roomName.getActionMap().put("ENTER", crtRoom);
            createBtn.addActionListener(crtRoom);
        }
    }
    private class JTableMouseListener implements MouseListener {
        SocketHandler handler;
        Values.WriteValues writeValues;
        Values.ReadValues readValues;
        JTableMouseListener(SocketHandler handler, Values.WriteValues writeValues, Values.ReadValues readValues) {
            this.handler = handler;
            this.writeValues = writeValues;
            this.readValues = readValues;
        }
        @Override
        public void mouseClicked(MouseEvent e) {
            if(writeValues.msgType == MsgType.MIN_MSG_VALUE) {
                JTable table = (JTable) e.getSource();
                int row = table.getSelectedRow();
                DefaultTableModel model = (DefaultTableModel) table.getModel();
                if (model.getValueAt(row, 0) != null) {
                    writeValues.msgType = MsgType.MSG_JOIN_ROOM;
                    values.staticValues.roomId = (int) model.getValueAt(row, 0);
                    values.staticValues.roomname = (String) model.getValueAt(row, 1);
                    values.writeValues.writeListener.actionPerformed(null);
                    setVisible(false);
                    new Waiting(handler, values);
                }
            }
        }
        @Override
        public void mousePressed(MouseEvent e) {}
        @Override
        public void mouseReleased(MouseEvent e) {}
        @Override
        public void mouseEntered(MouseEvent e) {}
        @Override
        public void mouseExited(MouseEvent e) {}
    }
}
