import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

class Chat extends JPanel implements ActionListener{

    private JPanel displayPanel;
    private JPanel inputPanel;
    private JTextArea display;
    private JTextField input = new JTextField(30);

    public Chat(CenterPanel centerPanel) {
        setFocusable(false);
        setSize(600, 510);
        displayPanel = new JPanel();
        displayPanel.setLayout(new FlowLayout());
        display = new JTextArea(15, 30);
        Font displayFont = new Font("맑은 고딕", Font.BOLD, 20);
        display.setFont(displayFont);
        display.setEditable(false);
        display.setFocusable(false);
// textarea를 스크롤이 되도록 변경
        JScrollPane scroll = new JScrollPane(display);
        displayPanel.add(scroll);
        inputPanel = new JPanel();
        inputPanel.setLayout(new FlowLayout());
        Font inputFont = new Font("맑은 고딕", Font.BOLD, 20);
        input.setFont(inputFont);
        input.setFocusable(true);
        input.addActionListener(this);
        input.addFocusListener(new FocusListener() {
            @Override
            public void focusGained(FocusEvent e) {
                centerPanel.actionPerformed(null);
            }

            @Override
            public void focusLost(FocusEvent e) {
            }
        });
        inputPanel.add(input);
        /*
         * Frame에 두개의 panel을 붙이기
         */
        this.setLayout(new BorderLayout());
        this.add(displayPanel, BorderLayout.CENTER);
        this.add(inputPanel, BorderLayout.SOUTH);
    }
    @Override
    public void actionPerformed(ActionEvent e) {
        if(e.getSource() == input)
        {
            display.append( input.getText() + "\n");
            input.selectAll();
            input.setText(null);
        }
    }
}
